import argparse
import importlib.util
import os
import shutil

try:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('..', '..', 'Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)
except FileNotFoundError:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('..', 'Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-u', '--username', type=str, help='Username',
    default='market_data_feed')
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=True)
  parser.add_argument('-gu', '--glimpse_username', type=str,
    help='ASX Glimpse username.', default='')
  parser.add_argument('-gp', '--glimpse_password', type=str,
    help='ASX Glimpse password.', default='')
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['username'] = args.username
  variables['admin_password'] = args.password
  variables['glimpse_username'] = args.glimpse_username
  variables['glimpse_password'] = args.glimpse_password
  for filename in os.listdir('.'):
    default_path = os.path.join(filename, 'config.default.yml')
    if filename.startswith('asxitch_') and os.path.isdir(filename) and \
        os.path.isfile(default_path):
      with open(default_path, 'r+') as file:
        source = setup_utils.translate(file.read(), variables)
        file.seek(0)
        file.write(source)
        file.truncate()
        shutil.move(default_path, os.path.join(filename, 'config.yml'))


if __name__ == '__main__':
  main()
