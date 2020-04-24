import argparse
import importlib.util
from mock import patch
import os
import shutil
import socket


def get_ip():
  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as test_socket:
    try:
      test_socket.connect(('10.255.255.255', 1))
      return test_socket.getsockname()[0]
    except:
      return '127.0.0.1'


def run_subscript(path, arguments):
  spec = importlib.util.spec_from_file_location('subscript', path)
  subscript = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(subscript)
  with patch.object(sys, 'argv', [path] + arguments):
    subscript.main()


def needs_quotes(value):
  special_characters = [':', '{', '}', '[', ']', ',', '&', '*', '#', '?', '|',
    '-', '<', '>', '=', '!', '%', '@', '\\']
  for c in value:
    if c in special_characters:
      return True
  return False


def translate(source, variables):
  for key in variables.keys():
    if needs_quotes(variables[key]):
      index = source.find('$' + key)
      while index != -1:
        c = source.rfind('\n', 0, index) + 1
        q = False
        while c < index:
          if source[c] == '\"':
            q = not q
          c += 1
        if q:
          source = source.replace('$' + key, '%s' % variables[key], 1)
        else:
          source = source.replace('$' + key, '"%s"' % variables[key], 1)
        index = source.find('$' + key, index + 1)
    else:
      source = source.replace('$' + key, '%s' % variables[key])
  return source


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=get_ip())
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    default='1234')
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    default='127.0.0.1:3306')
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', default='spireadmin')
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    default='spire')
  parser.add_argument('-gu', '--glimpse_username', type=str,
    help='ASX Glimpse username.', default='')
  parser.add_argument('-gp', '--glimpse_password', type=str,
    help='ASX Glimpse password.', default='')
  parser.add_argument('-cru', '--chia_retrans_username', type=str,
    help='CHIA retransmission username.', default='""')
  parser.add_argument('-crp', '--chia_retrans_password', type=str,
    help='CHIA retransmission password.', default='""')
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['global_address'] = \
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
  variables['glimpse_username'] = args.glimpse_username
  variables['glimpse_password'] = args.glimpse_password
  variables['chia_retransmission_username'] = args.chia_retrans_username
  variables['chia_retransmission_password'] = args.chia_retrans_password
  applications = [d for d in os.listdir('./') if os.path.isdir(d)]
  for feed in ['CseMarketDataFeedClient', 'CtaMarketDataFeedClient',
      'TmxIpMarketDataFeedClient', 'TmxTl1MarketDataFeedClient',
      'UtpMarketDataFeedClient']:
    if os.path.isdir(feed):
      os.chdir(feed)
      run_subscript('setup.py', ['-l', '"%s"' % variables['local_interface'],
        '-a', '"%s"' % variables['service_locator_address'],
        '-p', '"%s"' % variables['admin_password']])
      os.chdir('..')
  if os.path.isdir('AsxItchMarketDataFeedClient'):
    os.chdir('AsxItchMarketDataFeedClient')
    run_subscript('setup.py', ['-l', '"%s"' % variables['local_interface'],
      '-a', '"%s"' % variables['service_locator_address'],
      '-p', '"%s"' % variables['admin_password'],
      '-gu', '"%s"' % variables['glimpse_username'],
      '-gp', '"%s"' % variables['glimpse_password']])
    os.chdir('..')
  if os.path.isdir('ChiaMarketDataFeedClient'):
    os.chdir('ChiaMarketDataFeedClient')
    run_subscript('setup.py', ['-l', '"%s"' % variables['local_interface'],
      '-a', '"%s"' % variables['service_locator_address'],
      '-p', '"%s"' % variables['admin_password'],
      '-ru', '"%s"' % variables['chia_retransmission_username'],
      '-rp', '"%s"' % variables['chia_retransmission_password']])
    os.chdir('..')
  for application in applications:
    application_directory = os.path.join('.', application)
    files = [f for f in os.listdir(application_directory) if
      os.path.isfile(os.path.join(application_directory, f)) and
      f.find('.default.') != -1]
    for file in files:
      file_path = os.path.join(application_directory, file)
      destination_path = file_path.replace('.default.', '.')
      shutil.move(file_path, destination_path)
      with open(destination_path, 'r+') as file:
        source = translate(file.read(), variables)
        file.seek(0)
        file.write(source)
        file.truncate()


if __name__ == '__main__':
  main()
