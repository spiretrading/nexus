import argparse
import importlib.util
import os
from pathlib import Path
import subprocess
import sys

directory = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('setup_utils',
  directory / 'Python' / 'setup_utils.py')
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def create_symlink(source, target):
  if sys.platform == 'win32':
    subprocess.run(['cmd', '/c', 'mklink', '/j', source,
      str(Path(target).resolve())], check=True, stdout=subprocess.DEVNULL)
  else:
    os.symlink(target, source, target_is_directory=True)


def make_sub_args(arg_vars, *args):
  sub_args = []
  for arg in args:
    if arg_vars[arg] is not None:
      sub_args.append('--' + arg + '=' + str(arg_vars[arg]))
  return sub_args


def find_application(root, application):
  path = root / application / 'Application'
  if not (path / 'setup.py').is_file():
    path = root / application
  return path


def setup_application(root, application, arg_vars, *args):
  root_path = os.getcwd()
  try:
    os.chdir(find_application(root, application))
    print('Setting up ' + application, flush=True)
    setup_utils.run_subscript('setup.py', make_sub_args(arg_vars, *args))
  finally:
    os.chdir(root_path)


def setup_server(root, server, arg_vars):
  setup_application(root, server, arg_vars,
    'local', 'world', 'address', 'password')


def setup_server_with_mysql(root, server, arg_vars):
  setup_application(root, server, arg_vars,
    'local', 'world', 'address', 'password', 'mysql_address', 'mysql_username',
    'mysql_password', 'mysql_schema')


def setup_service_locator(root, arg_vars):
  setup_application(root, 'ServiceLocator', arg_vars, 'local', 'world',
    'mysql_address', 'mysql_username', 'mysql_password', 'mysql_schema')


def setup_beam(root, dependencies):
  for beam_service in ['ServiceLocator', 'UidServer']:
    path = root / beam_service
    if not os.path.lexists(path):
      create_symlink(str(path),
        str(dependencies / 'Beam' / 'Applications' / beam_service))


def main():
  parser = argparse.ArgumentParser(
    description='Set up Nexus application configurations.')
  parser.add_argument('--directory', type=Path, default=directory,
    help='Applications directory to configure (defaults to this script).')
  parser.add_argument('--dependencies', type=Path,
    help='Dependencies directory containing Beam.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    required=False)
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=True)
  parser.add_argument('-ma', '--mysql_address', type=str,
    help='MySQL address.', required=False)
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', required=False)
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    required=False)
  arg_vars = vars(parser.parse_args())
  root = arg_vars['directory'].resolve()
  dependencies = arg_vars['dependencies']
  if dependencies is None:
    dependencies = root.parent / 'Nexus' / 'Dependencies'
  dependencies = dependencies.resolve()
  if arg_vars['mysql_password'] is None:
    arg_vars['mysql_password'] = arg_vars['password']
  mysql_servers = ['UidServer', 'AdministrationServer', 'ComplianceServer',
    'MarketDataServer', 'RiskServer', 'SimulationOrderExecutionServer']
  servers = ['ChartingServer', 'DefinitionsServer', 'MarketDataRelayServer',
    'ReplayMarketDataFeedClient', 'SimulationMarketDataFeedClient', 'WebPortal']
  for application in ['ServiceLocator'] + mysql_servers + servers:
    path = find_application(root, application)
    if application in ['ServiceLocator', 'UidServer'] and \
        not os.path.lexists(root / application):
      path = find_application(dependencies / 'Beam' / 'Applications',
        application)
    if not (path / 'setup.py').is_file():
      parser.error(
        'Missing application setup script: ' + str(path / 'setup.py'))
  setup_beam(root, dependencies)
  setup_service_locator(root, arg_vars)
  for server in mysql_servers:
    setup_server_with_mysql(root, server, arg_vars)
  for server in servers:
    setup_server(root, server, arg_vars)


if __name__ == '__main__':
  main()
