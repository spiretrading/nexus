import argparse
import importlib.util
from pathlib import Path
import os
import shutil

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
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=True)
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['global_interface'] = \
    variables['local_interface'] if args.world is None else args.world
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['admin_password'] = args.password
  with open(directory / 'config.default.yml', encoding='utf-8') as file:
    source = setup_utils.translate(file.read(), variables)
  with open('config.yml', 'w', encoding='utf-8') as file:
    file.write(source)
  for file in ['countries.yml', 'currencies.yml', 'date_time_zonespec.csv',
      'destinations.yml', 'venues.yml']:
    if not os.path.exists(file):
      base, ext = os.path.splitext(file)
      shutil.copy(directory / (base + '.default' + ext), file)

if __name__ == '__main__':
  main()
