"""Build a site with a correct base URL; run only in a read-only build job."""
import argparse
from pathlib import Path

from mkdocs.commands.build import build
from mkdocs.config import load_config

parser = argparse.ArgumentParser()
parser.add_argument('--pr', type=int)
parser.add_argument('--output', required=True)
args = parser.parse_args()
if args.pr is not None and args.pr < 1:
    parser.error('--pr must be positive')
url = 'https://idle-lab.github.io/'
if args.pr:
    url += f'pr-preview/pr-{args.pr}/'
config = load_config(site_url=url, site_dir=str(Path(args.output).resolve()))
build(config, dirty=False)
