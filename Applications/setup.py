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


def setup_server_with_mysql(root, server, arg_vars, *args):
  setup_application(root, server, arg_vars,
    'local', 'world', 'address', 'password', 'mysql_address', 'mysql_username',
    'mysql_password', 'mysql_schema', *args)


def setup_service_locator(root, arg_vars):
  setup_application(root, 'ServiceLocator', arg_vars, 'local', 'world',
    'mysql_address', 'mysql_username', 'mysql_password', 'mysql_schema')


def setup_beam(root, dependencies):
  for beam_service in ['ServiceLocator', 'UidServer']:
    path = root / beam_service
    if not os.path.lexists(path):
      create_symlink(str(path),
        str(dependencies / 'Beam' / 'Applications' / beam_service))


def setup_feed(root, application, arg_vars, prefix, options):
  feed_args = arg_vars.copy()
  feed_args['username'] = arg_vars['feed_username']
  for option in options:
    feed_args[option] = arg_vars[prefix + option]
  setup_application(root, application, feed_args,
    'local', 'address', 'username', 'password', *options)


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
  parser.add_argument('--feed_username', default='market_data_feed',
    help='Service locator username for exchange feeds.')
  asx_options = ['glimpse_username', 'glimpse_password']
  cxa_options = ['retransmission_address', 'retransmission_session_sub_id',
    'retransmission_username', 'retransmission_password', 'spin_address1',
    'spin_address2', 'spin_session_sub_id', 'spin_username', 'spin_password']
  for section in ['retransmission', 'spin']:
    for field in ['address', 'session_sub_id', 'username', 'password']:
      for unit in [1, 2]:
        option = f'{section}_{field}{unit}'
        if option not in cxa_options:
          cxa_options.append(option)
  feeds = {
    'AsxTradeItchMarketDataFeedClient': ('asx_', asx_options),
    'CxaPitchMarketDataFeedClient': ('cxa_', cxa_options),
    'OtcLinkMarketDataFeedClient': ('otc_', ['sender']),
    'TmxIpMarketDataFeedClient': ('', [])
  }
  for prefix, options in feeds.values():
    for option in options:
      names = ['--' + prefix + option]
      if prefix == 'cxa_' and option == 'retransmission_username':
        names.extend(['-cru', '--cxa_retrans_username'])
      elif prefix == 'cxa_' and option == 'retransmission_password':
        names.extend(['-crp', '--cxa_retrans_password'])
      parser.add_argument(*names,
        help=prefix[:-1].upper() + ' ' + option.replace('_', ' ') + '.')
  oasis_options = ['asx_settings', 'serenity_settings', 'fee_table',
    'tsx_etfs', 'tsx_interlisted', 'nex_listed']
  for option in oasis_options:
    parser.add_argument('--oasis_' + option, type=Path,
      help='Oasis ' + option.replace('_', ' ') + ' file to install.')
  arg_vars = vars(parser.parse_args())
  root = arg_vars['directory'].resolve()
  dependencies = arg_vars['dependencies']
  if dependencies is None:
    dependencies = root.parent / 'Nexus' / 'Dependencies'
  dependencies = dependencies.resolve()
  if arg_vars['mysql_password'] is None:
    arg_vars['mysql_password'] = arg_vars['password']
  if bool(arg_vars['asx_glimpse_username']) != \
      bool(arg_vars['asx_glimpse_password']):
    parser.error('Supply both the ASX Glimpse username and password.')
  for option in oasis_options:
    key = 'oasis_' + option
    if arg_vars[key] is not None:
      arg_vars[key] = arg_vars[key].resolve()
      if not arg_vars[key].is_file():
        parser.error('Missing Oasis input file: ' + str(arg_vars[key]))
  mysql_servers = ['UidServer', 'AdministrationServer', 'ComplianceServer',
    'MarketDataServer', 'OasisOrderExecutionServer', 'RiskServer',
    'SimulationOrderExecutionServer']
  servers = ['ChartingServer', 'DefinitionsServer', 'MarketDataRelayServer',
    'ReplayMarketDataFeedClient', 'SimulationMarketDataFeedClient', 'WebPortal']
  for application in ['ServiceLocator'] + mysql_servers + servers + list(feeds):
    path = find_application(root, application)
    if application in ['ServiceLocator', 'UidServer'] and \
        not os.path.lexists(root / application):
      path = find_application(dependencies / 'Beam' / 'Applications',
        application)
    if not (path / 'setup.py').is_file():
      parser.error(
        'Missing application setup script: ' + str(path / 'setup.py'))
  if not arg_vars['otc_sender']:
    parser.error('--otc_sender is required for the Trades reference snapshot.')
  if '\x01' in arg_vars['otc_sender']:
    parser.error('--otc_sender must not contain the FIX SOH delimiter.')
  setup_beam(root, dependencies)
  setup_service_locator(root, arg_vars)
  for server in mysql_servers:
    if server == 'OasisOrderExecutionServer':
      oasis_args = arg_vars.copy()
      for option in oasis_options:
        oasis_args[option] = arg_vars['oasis_' + option]
      setup_server_with_mysql(root, server, oasis_args, *oasis_options)
    else:
      setup_server_with_mysql(root, server, arg_vars)
  for server in servers:
    setup_server(root, server, arg_vars)
  for feed, (prefix, options) in feeds.items():
    setup_feed(root, feed, arg_vars, prefix, options)


if __name__ == '__main__':
  main()
