import importlib.util
import json
import os
from pathlib import Path
import stat
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile

SPEC = importlib.util.spec_from_file_location('publisher', Path(__file__).parents[1] / 'scripts/publish_site.py')
p = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(p)


def write(root, name, text='content'):
    path = Path(root) / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return path


class SiteTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.site = self.root / 'source'
        write(self.site, 'index.html', 'new')
        write(self.site, 'assets/a.css', 'css')

    def tearDown(self):
        self.tmp.cleanup()

    def test_production_preserves_preview_cname_and_git(self):
        target = self.root / 'published'
        write(target, '.git/config', 'git')
        write(target, 'pr-preview/pr-8/index.html', 'preview')
        write(target, 'CNAME', 'example.com')
        write(target, 'old.html')
        p.apply_site(target, self.site)
        self.assertEqual((target / 'index.html').read_text(), 'new')
        self.assertTrue((target / 'pr-preview/pr-8/index.html').exists())
        self.assertTrue((target / '.git/config').exists())
        self.assertEqual((target / 'CNAME').read_text(), 'example.com')
        self.assertFalse((target / 'old.html').exists())
        self.assertTrue((target / '.nojekyll').exists())

    def test_preview_only_replaces_own_namespace(self):
        target = self.root / 'published'
        write(target, 'index.html', 'production')
        write(target, 'pr-preview/pr-8/obsolete.html')
        write(target, 'pr-preview/pr-9/index.html', 'other')
        p.apply_site(target, self.site, 8)
        self.assertEqual((target / 'index.html').read_text(), 'production')
        self.assertFalse((target / 'pr-preview/pr-8/obsolete.html').exists())
        self.assertEqual((target / 'pr-preview/pr-9/index.html').read_text(), 'other')

    def test_production_cannot_own_preview_namespace(self):
        write(self.site, 'pr-preview/pr-8/index.html')
        with self.assertRaises(ValueError):
            p.apply_site(self.root, self.site)

    def test_pr_number_validation(self):
        for number in [0, -1, '../8', '8']:
            with self.assertRaises(ValueError):
                p.apply_site(self.root, self.site, number)

    def test_cleanup_is_scoped_and_idempotent(self):
        write(self.root, 'pr-preview/pr-8/index.html')
        write(self.root, 'pr-preview/pr-9/index.html')
        write(self.root, 'pr-preview/README.txt')
        p.prune_previews(self.root, {8})
        p.prune_previews(self.root, {8})
        self.assertTrue((self.root / 'pr-preview/pr-8').exists())
        self.assertFalse((self.root / 'pr-preview/pr-9').exists())
        self.assertTrue((self.root / 'pr-preview/README.txt').exists())

    def archive(self, name, mode=None):
        archive = self.root / 'site.zip'
        with zipfile.ZipFile(archive, 'w') as zf:
            zf.writestr('index.html', 'hello')
            info = zipfile.ZipInfo(name)
            if mode is not None:
                info.external_attr = mode << 16
            zf.writestr(info, 'payload')
        return archive

    def test_zip_traversal_and_git_files_rejected(self):
        for name in ['../escape', '/absolute', r'a\b', '.git/config', 'a/.GIT/x', '.gitattributes']:
            with self.subTest(name=name), self.assertRaises(ValueError):
                p.extract_site(self.archive(name), self.root / 'extract')
        self.assertFalse((self.root / 'extract').exists())

    def test_zip_symlinks_and_special_files_rejected(self):
        for mode in [stat.S_IFLNK | 0o777, stat.S_IFIFO | 0o644]:
            with self.assertRaises(ValueError):
                p.extract_site(self.archive('link', mode), self.root / 'extract')

    def test_zip_limits_before_extraction(self):
        with patch.object(p, 'MAX_SITE', 5), self.assertRaises(ValueError):
            p.extract_site(self.archive('file.txt'), self.root / 'extract')
        self.assertFalse((self.root / 'extract').exists())

    def test_valid_zip_and_local_symlink_rejection(self):
        target = self.root / 'extract'
        p.extract_site(self.archive('assets/a.css'), target)
        self.assertEqual((target / 'index.html').read_text(), 'hello')
        (self.site / 'link').symlink_to(self.root)
        with self.assertRaises(ValueError):
            p.validate_site(self.site)

    def test_duplicate_zip_entry_rejected(self):
        import warnings
        archive = self.root / 'duplicate.zip'
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(archive, 'w') as zf:
                zf.writestr('index.html', 'first')
                zf.writestr('index.html', 'second')
        with self.assertRaises(ValueError):
            p.extract_site(archive, self.root / 'extract')



class CandidateTests(unittest.TestCase):
    def setUp(self):
        self.pr = {'number': 8, 'state': 'open', 'head': {
            'repo': {'full_name': 'idle-lab/idle-lab.github.io'}, 'sha': 'a'*40, 'ref': 'docs/test'},
            'merge_commit_sha': 'b'*40, 'base': {'ref': 'master'}}
        self.run = {'id': 4, 'workflow_id': 5, 'event': 'pull_request', 'conclusion': 'success',
                    'head_repository': {'full_name': 'idle-lab/idle-lab.github.io'},
                    'head_sha': 'a'*40, 'head_branch': 'docs/test',
                    'pull_requests': [{'number': 8, 'head': {'sha': 'a'*40}}]}
        self.workflow = {'path': '.github/workflows/PreviewBuild.yml'}
        outer = self
        class API:
            repo = 'idle-lab/idle-lab.github.io'
            def api(self, path):
                return outer.run if path.startswith('actions/runs') else (
                    outer.workflow if path.startswith('actions/workflows') else outer.pr)
        self.api = API()
        self.event = {'workflow_run': {'id': 4}, 'repository': {'default_branch': 'master'}}

    def test_current_same_repo_candidate(self):
        self.assertIsNotNone(p.preview_candidate(self.api, self.event))
        self.run['head_sha'] = 'b'*40
        self.assertIsNotNone(p.preview_candidate(self.api, self.event))

    def test_fork_wrong_workflow_and_stale_sha_rejected(self):
        for obj, key, value in [
                (self.run, 'event', 'push'), (self.run, 'conclusion', 'failure'),
                (self.run, 'head_sha', 'c'*40),
                (self.run['head_repository'], 'full_name', 'other/fork'),
                (self.pr['head']['repo'], 'full_name', 'other/fork'),
                (self.pr['head'], 'sha', 'd'*40), (self.pr, 'state', 'closed'),
                (self.pr['base'], 'ref', 'other-branch'),
                (self.workflow, 'path', '.github/workflows/attacker.yml')]:
            original = obj[key]
            obj[key] = value
            with self.subTest(key=key, value=value):
                self.assertIsNone(p.preview_candidate(self.api, self.event))
            obj[key] = original


class PublishFlowTests(unittest.TestCase):
    def test_closed_pr_after_push_runs_cleanup_without_comment(self):
        pr = {'number': 8, 'head': {'sha': 'a' * 40}}
        run = {'id': 4}
        class API:
            def pages(self, *args):
                return [{'id': 42, 'name': 'site-preview-8-' + 'a' * 40, 'expired': False}]
            def download(self, *args): pass
            def api(self, path): return {'state': 'closed', 'head': {'sha': 'a' * 40}}
        api = API()
        with tempfile.TemporaryDirectory() as tmp:
            event = write(tmp, 'event.json', '{}')
            with patch.dict(os.environ, GITHUB_EVENT_NAME='workflow_run', GITHUB_EVENT_PATH=str(event)), patch.object(p, 'GitHub', return_value=api), patch.object(p, 'preview_candidate', return_value=(pr, run)), patch.object(p, 'extract_site'), patch.object(p, 'publish', return_value=True) as publish, patch.object(p, 'comment') as comment:
                p.main('preview')
                self.assertEqual(publish.call_count, 2)
                self.assertEqual(publish.call_args.args, (api, {}))
                comment.assert_not_called()

    def test_duplicate_artifacts_fail_closed(self):
        pr = {'number': 8, 'head': {'sha': 'a' * 40}}
        class API:
            def pages(self, *args):
                return [{'id': n, 'name': 'site-preview-8-' + 'a' * 40, 'expired': False} for n in [1, 2]]
        with tempfile.TemporaryDirectory() as tmp:
            event = write(tmp, 'event.json', '{}')
            with patch.dict(os.environ, GITHUB_EVENT_NAME='workflow_run', GITHUB_EVENT_PATH=str(event)), patch.object(p, 'GitHub', return_value=API()), patch.object(p, 'preview_candidate', return_value=(pr, {'id': 4})), patch.object(p, 'publish') as publish, self.assertRaises(ValueError):
                p.main('preview')
            publish.assert_not_called()


class GitIntegrationTests(unittest.TestCase):
    def test_non_force_retry_reapplies_and_preserves_concurrent_preview(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            remote, seed, source = root / 'remote.git', root / 'seed', root / 'source'
            remote.mkdir(); seed.mkdir()
            subprocess.run(['git', 'init', '--bare', '--quiet', str(remote)], check=True)
            original_git = p.git
            original_git(seed, 'init', '--quiet', '-b', 'gh-pages')
            original_git(seed, 'remote', 'add', 'origin', str(remote))
            write(seed, 'index.html', 'old production')
            original_git(seed, 'add', '.')
            original_git(seed, '-c', 'user.name=Tests', '-c', 'user.email=tests@example.invalid', 'commit', '-m', 'seed')
            original_git(seed, 'push', 'origin', 'gh-pages')
            write(source, 'index.html', 'new production')
            class API:
                repo = 'idle-lab/idle-lab.github.io'
                token = 'test-token-not-real'
                def pages(self, path):
                    return [{'number': 9, 'head': {'repo': {'full_name': self.repo}}}]
                def api(self, path):
                    return {'sha': 'current'}
            collided = []
            def intercept(cwd, *args, **kwargs):
                if args[:3] == ('remote', 'add', 'origin'):
                    args = (*args[:3], str(remote))
                if args[0] == 'push' and not collided:
                    collided.append(True)
                    write(seed, 'pr-preview/pr-9/index.html', 'concurrent preview')
                    original_git(seed, 'add', '.')
                    original_git(seed, '-c', 'user.name=Tests', '-c', 'user.email=tests@example.invalid', 'commit', '-m', 'other preview')
                    original_git(seed, 'push', 'origin', 'gh-pages')
                self.assertNotIn('--force', args)
                return original_git(cwd, *args, **kwargs)
            with patch.object(p, 'git', side_effect=intercept), patch.object(p.time, 'sleep'):
                with patch.dict(os.environ, GITHUB_SHA='stale'):
                    self.assertFalse(p.publish(API(), {'repository': {'default_branch': 'master'}}, source, production=True))
                    self.assertFalse(collided)
                with patch.dict(os.environ, GITHUB_SHA='current'):
                    self.assertTrue(p.publish(API(), {'repository': {'default_branch': 'master'}}, source, production=True))
            self.assertTrue(collided)
            self.assertEqual(original_git(root, '--git-dir', str(remote), 'show', 'gh-pages:index.html').stdout, 'new production')
            self.assertEqual(original_git(root, '--git-dir', str(remote), 'show', 'gh-pages:pr-preview/pr-9/index.html').stdout, 'concurrent preview')


if __name__ == '__main__':
    unittest.main()
