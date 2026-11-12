"""Publish static artifacts, never execute their contents or overwrite other previews.

Privileged callers must use this script from the reviewed default branch. No
third-party Python packages, persistent credentials, force pushes, or hooks.
"""
import base64
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import subprocess
import tempfile
import time
import urllib.request
import zipfile

BASE_URL = 'https://idle-lab.github.io/'
PREVIEWS = 'pr-preview'
BRANCH = 'gh-pages'
MAX_SITE = 200 * 1024 * 1024
MAX_TOTAL = 900 * 1024 * 1024  # Leave headroom below Pages' 1 GB limit.
MAX_FILES = 20000
MARKER = '<!-- mkdocs-pr-preview -->'
BLOCKED = {'.git', '.github', '.gitmodules', '.gitattributes', '.gitignore'}


def safe_parts(name):
    path = PurePosixPath(name)
    if (path.is_absolute() or '\\' in name or not path.parts
            or any(p in ('..', '.') or p.lower() in BLOCKED for p in path.parts)
            or any(ord(c) < 32 for c in name)):
        raise ValueError(f'Unsafe artifact path: {name!r}')
    return path.parts


def extract_site(archive, target):
    """Validate all entries before extraction; do not follow ZIP links."""
    target = Path(target)
    with zipfile.ZipFile(archive) as zf:
        infos = zf.infolist()
        if len(infos) > MAX_FILES or sum(i.file_size for i in infos) > MAX_SITE:
            raise ValueError('Artifact exceeds size or file-count limit')
        seen = set()
        for item in infos:
            parts = safe_parts(item.filename)
            key = '/'.join(parts)
            mode = item.external_attr >> 16
            kind = stat.S_IFMT(mode)
            if key in seen or kind not in (0, stat.S_IFREG, stat.S_IFDIR):
                raise ValueError('Duplicate or non-regular artifact entry')
            seen.add(key)
        for item in infos:
            dest = target.joinpath(*safe_parts(item.filename))
            if item.is_dir():
                dest.mkdir(parents=True, exist_ok=True)
            else:
                dest.parent.mkdir(parents=True, exist_ok=True)
                with zf.open(item) as src, dest.open('xb') as out:
                    shutil.copyfileobj(src, out)
    validate_site(target)


def validate_site(site):
    site = Path(site)
    if not (site / 'index.html').is_file() or site.is_symlink():
        raise ValueError('Site must contain index.html')
    count = total = 0
    for p in site.rglob('*'):
        safe_parts(p.relative_to(site).as_posix())
        if p.is_symlink() or not (p.is_file() or p.is_dir()):
            raise ValueError('Site contains a link or special file')
        if p.is_file():
            count += 1
            total += p.stat().st_size
    if count > MAX_FILES or total > MAX_SITE:
        raise ValueError('Site exceeds size or file-count limit')


def remove(path):
    if path.is_dir() and not path.is_symlink():
        shutil.rmtree(path)
    elif path.exists() or path.is_symlink():
        path.unlink()


def apply_site(root, source, number=None):
    """Replace one namespace only; root deploy preserves previews and CNAME."""
    root, source = Path(root), Path(source)
    validate_site(source)
    if number is not None:
        if not isinstance(number, int) or number < 1:
            raise ValueError('Invalid PR number')
        target = root / PREVIEWS / f'pr-{number}'
        remove(target)
        target.parent.mkdir(exist_ok=True)
        shutil.copytree(source, target)
    else:
        if (source / PREVIEWS).exists():
            raise ValueError('Production artifact may not contain preview namespace')
        for p in root.iterdir():
            if p.name not in ('.git', PREVIEWS, 'CNAME'):
                remove(p)
        shutil.copytree(source, root, dirs_exist_ok=True)
    (root / '.nojekyll').touch()


def prune_previews(root, open_numbers):
    parent = Path(root) / PREVIEWS
    if parent.is_symlink():
        raise ValueError('Preview namespace cannot be a symlink')
    if parent.exists():
        for p in parent.iterdir():
            match = re.fullmatch(r'pr-([1-9][0-9]*)', p.name)
            if match and int(match[1]) not in open_numbers:
                remove(p)


class GitHub:
    def __init__(self):
        self.repo = os.environ['GITHUB_REPOSITORY']
        if self.repo != 'idle-lab/idle-lab.github.io':
            raise ValueError('Unexpected repository')
        self.token = os.environ['GH_TOKEN']
        self.prefix = f'https://api.github.com/repos/{self.repo}/'

    def api(self, path, data=None, method=None):
        req = urllib.request.Request(self.prefix + path,
            data=None if data is None else json.dumps(data).encode(),
            method=method,
            headers={'Authorization': f'Bearer {self.token}',
                     'Accept': 'application/vnd.github+json',
                     'X-GitHub-Api-Version': '2022-11-28'})
        with urllib.request.urlopen(req, timeout=60) as response:
            return json.load(response)

    def pages(self, path, key=None):
        items = []
        for page in range(1, 101):
            separator = '&' if '?' in path else '?'
            result = self.api(f'{path}{separator}per_page=100&page={page}')
            batch = result[key] if key else result
            items.extend(batch)
            if len(batch) < 100:
                return items
        raise ValueError('API pagination limit reached')

    def download(self, artifact_id, destination):
        # Follow GitHub's artifact-storage redirect without forwarding the token.
        class NoRedirect(urllib.request.HTTPRedirectHandler):
            def redirect_request(self, *args, **kwargs):
                return None
        req = urllib.request.Request(self.prefix + f'actions/artifacts/{artifact_id}/zip',
            headers={'Authorization': f'Bearer {self.token}'})
        try:
            urllib.request.build_opener(NoRedirect()).open(req, timeout=60)
        except urllib.error.HTTPError as e:
            if e.code != 302:
                raise
            location = e.headers['Location']
        else:
            raise ValueError('Expected GitHub artifact redirect')
        if not location.startswith('https://'):
            raise ValueError('Insecure artifact redirect')
        with urllib.request.urlopen(location, timeout=60) as src, open(destination, 'wb') as out:
            remaining = MAX_SITE + 1024 * 1024
            while chunk := src.read(min(1024 * 1024, remaining + 1)):
                remaining -= len(chunk)
                if remaining < 0:
                    raise ValueError('Compressed artifact exceeds limit')
                out.write(chunk)


def preview_candidate(api, event):
    run = api.api(f"actions/runs/{int(event['workflow_run']['id'])}")
    workflow = api.api(f"actions/workflows/{int(run['workflow_id'])}")
    if (workflow['path'] != '.github/workflows/PreviewBuild.yml'
            or run['event'] != 'pull_request' or run['conclusion'] != 'success'
            or run['head_repository']['full_name'] != api.repo):
        return None
    for candidate in run['pull_requests']:
        pr = api.api(f"pulls/{int(candidate['number'])}")
        if (pr['state'] == 'open' and pr['head']['repo']
                and pr['head']['repo']['full_name'] == api.repo
                and pr['head']['sha'] == candidate['head']['sha']
                and pr['head']['ref'] == run['head_branch']
                and run['head_sha'] in (pr['head']['sha'], pr['merge_commit_sha'])
                and pr['base']['ref'] == event['repository']['default_branch']):
            return pr, run
    return None


def git(root, *args, env=None, check=True):
    result = subprocess.run(['git', '-c', 'core.hooksPath=/dev/null', *args],
        cwd=root, env=env, capture_output=True, text=True)
    if check and result.returncode:
        # Do not log credential-bearing commands or remote output.
        raise RuntimeError(f'Git operation {args[0]} failed with {result.returncode}')
    return result


def publish(api, event, source=None, pr=None, production=False):
    """Optimistic concurrency: fetch fresh state and reapply after any push race."""
    env = os.environ.copy()
    credential = base64.b64encode(f'x-access-token:{api.token}'.encode()).decode()
    env.update(GIT_CONFIG_COUNT='1', GIT_CONFIG_KEY_0='http.https://github.com/.extraheader',
               GIT_CONFIG_VALUE_0=f'AUTHORIZATION: basic {credential}', GIT_TERMINAL_PROMPT='0')
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        git(root, 'init', '--quiet')
        git(root, 'remote', 'add', 'origin', f'https://github.com/{api.repo}.git')
        for attempt in range(5):
            git(root, 'fetch', '--depth=1', 'origin', BRANCH, env=env)
            git(root, 'checkout', '-B', BRANCH, 'FETCH_HEAD')
            git(root, 'clean', '-fdx')
            open_prs = api.pages('pulls?state=open')
            open_numbers = {p['number'] for p in open_prs if p['head']['repo']
                            and p['head']['repo']['full_name'] == api.repo}
            prune_previews(root, open_numbers)
            if source is not None:
                apply_site(root, source, pr['number'] if pr else None)
            size = sum(p.stat().st_size for p in root.rglob('*')
                       if p.is_file() and '.git' not in p.relative_to(root).parts)
            if size > MAX_TOTAL:
                raise ValueError('Combined Pages site exceeds 900 MiB; close old preview PRs')
            # Check immediately before committing/pushing, including every retry.
            if pr:
                current = api.api(f"pulls/{pr['number']}")
                if (current['state'] != 'open' or current['head']['sha'] != pr['head']['sha']
                        or current['head']['repo']['full_name'] != api.repo):
                    print('Skip stale or closed PR preview')
                    return False
            if production:
                branch = event['repository']['default_branch']
                current = api.api(f'commits/{branch}')
                if current['sha'] != os.environ['GITHUB_SHA']:
                    print('Skip superseded production build')
                    return False
            git(root, 'add', '--all')
            if not git(root, 'diff', '--cached', '--quiet', check=False).returncode:
                return True
            message = f"Deploy preview PR #{pr['number']} ({pr['head']['sha'][:12]})" if pr else (
                'Deploy production site; preserve PR previews' if production else 'Clean closed PR previews')
            git(root, '-c', 'user.name=github-actions[bot]', '-c',
                'user.email=41898282+github-actions[bot]@users.noreply.github.com',
                'commit', '-m', message)
            result = git(root, 'push', 'origin', f'HEAD:refs/heads/{BRANCH}', env=env, check=False)
            if result.returncode == 0:
                return True
            time.sleep(2 ** attempt)
        raise RuntimeError('Could not fast-forward gh-pages after five attempts; rerun workflow')


def comment(api, number, sha):
    body = (f'{MARKER}\n### 网站预览\n\n'
            f'[{BASE_URL}{PREVIEWS}/pr-{number}/]({BASE_URL}{PREVIEWS}/pr-{number}/)\n\n'
            f'对应 PR 提交 `{sha}`。构建产物已写入 `gh-pages`，Pages 发布后可访问，通常需要几分钟。'
            '\n\n这是公开预览；PR 更新后重新构建，关闭后删除。')
    comments = api.pages(f'issues/{number}/comments')
    existing = next((c for c in comments if c['user']['login'] == 'github-actions[bot]'
                     and c['body'].startswith(MARKER)), None)
    if existing:
        api.api(f"issues/comments/{existing['id']}", {'body': body}, 'PATCH')
    else:
        api.api(f'issues/{number}/comments', {'body': body}, 'POST')


def main(mode):
    api = GitHub()
    event = json.loads(Path(os.environ['GITHUB_EVENT_PATH']).read_text())
    if mode == 'production':
        publish(api, event, Path(os.environ['SITE_DIR']), production=True)
        return
    if os.environ['GITHUB_EVENT_NAME'] != 'workflow_run':
        publish(api, event)  # Cleanup only. No PR source or artifact is checked out.
        return
    selected = preview_candidate(api, event)
    if not selected:
        print('No eligible current same-repository PR; nothing published')
        return
    pr, run = selected
    name = f"site-preview-{pr['number']}-{pr['head']['sha']}"
    artifacts = [a for a in api.pages(f"actions/runs/{run['id']}/artifacts", 'artifacts')
                 if a['name'] == name and not a['expired']]
    if len(artifacts) != 1:
        raise ValueError('Expected exactly one artifact for this run, PR and head SHA')
    with tempfile.TemporaryDirectory() as directory:
        archive, site = Path(directory) / 'site.zip', Path(directory) / 'site'
        api.download(artifacts[0]['id'], archive)
        extract_site(archive, site)
        if publish(api, event, site, pr):
            current = api.api(f"pulls/{pr['number']}")
            if current['state'] == 'open' and current['head']['sha'] == pr['head']['sha']:
                comment(api, pr['number'], pr['head']['sha'])
            else:
                publish(api, event)  # Handle a close racing with our final push.


if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2 or sys.argv[1] not in ('production', 'preview'):
        raise SystemExit('Usage: publish_site.py production|preview')
    main(sys.argv[1])
