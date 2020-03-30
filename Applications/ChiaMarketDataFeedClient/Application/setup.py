import argparse
import os
import shutil
import subprocess
import sys

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode(
    'utf-8')

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
    default=call('hostname -I').strip())
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-u', '--username', type=str, help='Username',
    default='market_data_feed')
  parser.add_argument('-p', '--password', type=str, help='Password.',
    default='1234')
  parser.add_argument('-ru', '--retrans_username', type=str,
    help='Retransmission username.', default='""')
  parser.add_argument('-rp', '--retrans_password', type=str,
    help='Retransmission password.', default='""')
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['username'] = args.username
  variables['admin_password'] = args.password
  variables['chia_retransmission_username'] = args.retrans_username
  variables['chia_retransmission_password'] = args.retrans_password
  for filename in os.listdir('.'):
    default_path = os.path.join(filename, 'config.default.yml')
    if filename.startswith('chia_') and os.path.isdir(filename) and \
        os.path.isfile(default_path):
      with open(default_path, 'r+') as file:
        source = translate(file.read(), variables)
        file.seek(0)
        file.write(source)
        file.truncate()
        shutil.move(default_path, os.path.join(filename, 'config.yml'))

if __name__ == '__main__':
  main()
