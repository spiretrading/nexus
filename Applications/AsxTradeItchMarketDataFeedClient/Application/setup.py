import argparse
import importlib.util
from pathlib import Path

directory = Path(__file__).resolve().parent
helper_path = directory / 'setup_utils.py'
if not helper_path.is_file():
  helper_path = directory / '..' / '..' / 'Python' / 'setup_utils.py'
  if not helper_path.is_file():
    helper_path = directory / '..' / 'Python' / 'setup_utils.py'
spec = importlib.util.spec_from_file_location('setup_utils', helper_path)
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def remove_section(source, name):
  lines = []
  is_skipping = False
  for line in source.splitlines(keepends=True):
    if line.startswith(name + ':'):
      is_skipping = True
    elif is_skipping and line.strip() and not line[0].isspace():
      is_skipping = False
    if not is_skipping:
      lines.append(line)
  return ''.join(lines)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-a', '--address', type=str, help='Spire address.')
  parser.add_argument('-u', '--username', type=str, help='Username.',
    default='market_data_feed')
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=True)
  parser.add_argument('-gu', '--glimpse_username', type=str,
    help='ASX Glimpse username.', default='')
  parser.add_argument('-gp', '--glimpse_password', type=str,
    help='ASX Glimpse password.', default='')
  args = parser.parse_args()
  if bool(args.glimpse_username) != bool(args.glimpse_password):
    parser.error('Supply both the Glimpse username and password.')
  variables = {}
  variables['local_interface'] = args.local
  variables['service_locator_address'] = args.address
  if args.address is None:
    variables['service_locator_address'] = '%s:20000' % args.local
  variables['username'] = args.username
  variables['admin_password'] = args.password
  variables['glimpse_username'] = args.glimpse_username
  variables['glimpse_password'] = args.glimpse_password
  for folder in sorted(directory.glob('asx_partition*')):
    default_path = folder / 'config.default.yml'
    if not default_path.is_file():
      continue
    with open(default_path, encoding='utf-8') as file:
      source = file.read()
    if not args.glimpse_username:
      source = remove_section(source, 'glimpse')
    source = setup_utils.translate(source, variables)
    output_directory = Path(folder.name)
    output_directory.mkdir(exist_ok=True)
    with open(output_directory / 'config.yml', 'w', encoding='utf-8') as file:
      file.write(source)


if __name__ == '__main__':
  main()
