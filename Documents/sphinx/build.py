import errno
import json
import os
from pathlib import Path
import stat
import subprocess
import sys


MANIFEST = '.sphinx_generated_files.json'


def is_link(path):
  info = path.lstat()
  attributes = getattr(info, 'st_file_attributes', 0)
  reparse_point = getattr(stat, 'FILE_ATTRIBUTE_REPARSE_POINT', 0)
  return stat.S_ISLNK(info.st_mode) or attributes & reparse_point != 0


def collect(directory):
  entries = {}
  for root, directories, files in os.walk(directory):
    root = Path(root)
    directories[:] = [name for name in directories
      if not is_link(root / name)]
    for name in directories + files:
      path = root / name
      if path == directory / MANIFEST or is_link(path):
        continue
      entries[path.relative_to(directory).as_posix()] = path.is_dir()
  return entries


def clean(directory, generated):
  for name in sorted(generated, key=lambda name: len(Path(name).parts),
      reverse=True):
    path = Path(os.path.abspath(directory / name))
    if path == directory or not path.parent.resolve().is_relative_to(directory):
      raise ValueError('Invalid generated path: ' + name)
    if not path.exists() or is_link(path):
      continue
    if generated[name]:
      try:
        path.rmdir()
      except OSError as error:
        if error.errno not in (errno.ENOTEMPTY, errno.EEXIST):
          raise
    else:
      path.unlink()
  (directory / MANIFEST).unlink(missing_ok=True)


def main():
  if len(sys.argv) < 5:
    print('Usage: build.py sphinx-build target source output [options]',
      file=sys.stderr)
    return 1
  executable, target, source, output = sys.argv[1:5]
  directory = Path(output).resolve()
  manifest = directory / MANIFEST
  generated = {}
  if manifest.exists():
    generated = json.loads(manifest.read_text(encoding='utf-8'))
  if target == 'clean':
    clean(directory, generated)
    return 0
  command = [executable, '-M', target, source, output, *sys.argv[5:]]
  if target == 'help':
    return subprocess.call(command)
  previous = collect(directory)
  directory.mkdir(parents=True, exist_ok=True)
  try:
    return subprocess.call(command)
  finally:
    for name, is_directory in collect(directory).items():
      if name not in previous:
        generated[name] = is_directory
    manifest.write_text(json.dumps(generated, indent=2, sort_keys=True) + '\n',
      encoding='utf-8')


if __name__ == '__main__':
  try:
    sys.exit(main())
  except (OSError, ValueError) as error:
    print('Error: ' + str(error), file=sys.stderr)
    sys.exit(1)
