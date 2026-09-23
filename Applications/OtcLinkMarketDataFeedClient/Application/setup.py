import argparse
import getpass
import importlib.util
import os
from pathlib import Path
import sys

directory = Path(__file__).resolve().parent
helper_path = directory / 'setup_utils.py'
if not helper_path.is_file():
  helper_path = directory / '..' / '..' / 'Python' / 'setup_utils.py'
  if not helper_path.is_file():
    helper_path = directory / '..' / 'Python' / 'setup_utils.py'
spec = importlib.util.spec_from_file_location('setup_utils', helper_path)
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-u', '--username', type=str, help='Username',
    default='market_data_feed')
  parser.add_argument('-p', '--password', type=str,
    help='Password. Prompts when omitted unless --common-config is supplied.')
  parser.add_argument('--common-config', type=Path,
    help='Existing shared configuration to include without modifying it.')
  args = parser.parse_args()
  if args.common_config is not None:
    common_path = args.common_config.resolve()
    if not common_path.is_file():
      parser.error('--common-config must name an existing file.')
  else:
    common_path = Path('config.yml').resolve()
    if args.password is None:
      if not sys.stdin.isatty():
        parser.error('--password is required when generating a shared '
          'configuration noninteractively.')
      try:
        args.password = getpass.getpass('Spire password: ')
      except (EOFError, KeyboardInterrupt):
        parser.error('Password prompt cancelled.')
  variables = {}
  variables['local_interface'] = args.local
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['username'] = args.username
  variables['admin_password'] = args.password
  if args.common_config is None:
    with open(directory / 'config.default.yml', encoding='utf-8') as file:
      source = setup_utils.translate(file.read(), variables)
    with open(common_path, 'w', encoding='utf-8') as file:
      file.write(source)
  for folder in sorted(directory.glob('otcm_*')):
    default_path = folder / 'config.default.yml'
    if not default_path.is_file():
      continue
    with open(default_path, encoding='utf-8') as file:
      source = file.read()
    output_directory = Path(folder.name)
    try:
      common_reference = Path(os.path.relpath(common_path, output_directory))
    except ValueError:
      common_reference = common_path
    variables['common_config'] = common_reference.as_posix()
    source = setup_utils.translate(source, variables)
    output_directory.mkdir(exist_ok=True)
    with open(output_directory / 'config.yml', 'w', encoding='utf-8') as file:
      file.write(source)


if __name__ == '__main__':
  main()
