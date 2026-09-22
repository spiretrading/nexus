import argparse
import importlib.util
from pathlib import Path
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
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=True)
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    default='127.0.0.1:3306')
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', default='spireadmin')
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    default='spire')
  files = {
    'asx_settings': 'asx.cfg',
    'serenity_settings': 'serenity.cfg',
    'fee_table': 'fee_table.yml',
    'tsx_etfs': 'tsx_etfs.yml',
    'tsx_interlisted': 'tsx_interlisted.yml',
    'nex_listed': 'nex_listed.yml'
  }
  for option in files:
    parser.add_argument('--' + option, type=Path,
      help=option.replace('_', ' ') + ' file to install.')
  args = parser.parse_args()
  settings = {}
  for option, name in files.items():
    path = getattr(args, option)
    if path is not None:
      path = path.resolve()
      if not path.is_file():
        parser.error('Missing input file: ' + str(path))
    settings[name] = path
  variables = {}
  variables['local_interface'] = args.local
  variables['global_interface'] = \
    variables['local_interface'] if args.world is None else args.world
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['admin_password'] = args.password
  variables['mysql_address'] = args.mysql_address
  variables['mysql_username'] = args.mysql_username
  variables['mysql_password'] = \
    variables['admin_password'] if args.mysql_password is None else \
    args.mysql_password
  variables['mysql_schema'] = args.mysql_schema
  with open(directory / 'config.default.yml', encoding='utf-8') as file:
    source = setup_utils.translate(file.read(), variables)
  with open('config.yml', 'w', encoding='utf-8') as file:
    file.write(source)
  for name, path in settings.items():
    target = Path(name)
    if path is not None:
      if path != target.resolve():
        shutil.copyfile(path, target)
    elif not target.exists():
      default = directory / (target.stem + '.default' + target.suffix)
      if default.is_file():
        shutil.copyfile(default, target)
        if target.suffix == '.cfg':
          print('Created ' + name +
            '; configure its FIX connection settings before starting.')
      else:
        print('Supply ' + name + ' before starting OasisOrderExecutionServer.')


if __name__ == '__main__':
  main()
