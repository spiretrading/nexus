import argparse
import importlib.util
import os

spec = importlib.util.spec_from_file_location('setup_utils',
  os.path.join('Python', 'setup_utils.py'))
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def make_sub_args(arg_vars, *args):
  sub_args = []
  for arg in args:
    if arg_vars[arg]:
      sub_args += ['--' + arg, arg_vars[arg]]
  return sub_args


def setup_server(server, arg_vars):
  root_path = os.getcwd()
  try:
    os.chdir(os.path.join(server, 'Application'))
    setup_utils.run_subscript('setup.py', make_sub_args(arg_vars,
      'local', 'world', 'address', 'password'))
  finally:
    os.chdir(root_path)


def setup_server_with_mysql(server, arg_vars):
  root_path = os.getcwd()
  try:
    os.chdir(os.path.join(server, 'Application'))
    setup_utils.run_subscript('setup.py', make_sub_args(arg_vars,
      'local', 'world', 'address', 'password', 'mysql_address',
      'mysql_username', 'mysql_password', 'mysql_schema'))
  finally:
    os.chdir(root_path)


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
    required=False)
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', required=False)
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    required=False)
  arg_vars = vars(parser.parse_args())
  for server in ['AdministrationServer', 'ComplianceServer', 'MarketDataServer',
      'SimulationOrderExecutionServer']:
    setup_server_with_mysql(server, arg_vars)
  for server in ['ChartingServer', 'DefinitionsServer', 'MarketDataRelayServer',
      'ReplayMarketDataFeedClient', 'RiskServer',
      'SimulationMarketDataFeedClient', 'WebPortal']:
    setup_server(server, arg_vars)
  os.chdir(os.path.join('..', 'Nexus', 'Dependencies', 'Beam', 'Applications'))
  setup_utils.run_subscript('setup.py', make_sub_args(arg_vars,
    *[key for key in arg_vars.keys()]))


if __name__ == '__main__':
  main()
