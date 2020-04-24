import argparse
import importlib.machinery
import os

setup_utils = importlib.machinery.SourceFileLoader('setup_utils',
  os.path.join('..', 'Utilities', 'setup_utils.py')).load_module()


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    required=False)
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=False)
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    default='127.0.0.1:3306')
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', required=False)
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    required=False)
  args = parser.parse_args()
  os.chdir(os.path.join('AdministrationServer', 'Application'))
  sub_args = []
  if args.local:
    sub_args += ['-l', args.local]
  if args.password:
    sub_args += ['-p', args.password]
  if args.world:
    sub_args += ['-w', args.world]
  if args.address:
    sub_args += ['-a', args.address]
  if args.mysql_address:
    sub_args += ['-ma', args.mysql_address]
  if args.mysql_username:
    sub_args += ['-mu', args.mysql_username]
  if args.mysql_password:
    sub_args += ['-mp', args.mysql_password]
  if args.mysql_schema:
    sub_args += ['-ms', args.mysql_schema]
  setup_utils.run_subscript('setup.py', sub_args)


if __name__ == '__main__':
  main()
