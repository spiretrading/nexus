import argparse
import os
import shutil
import subprocess
import sys

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode(
    'utf-8')

def translate(source, variables):
  for key in variables.keys():
    source = source.replace('$' + key, '%s' % variables[key])
  return source

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2009 Eidolon Systems Ltd.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=call('hostname -I').strip())
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-u', '--username', type=str, help='Username',
    default='market_data_feed')
  parser.add_argument('-p', '--password', type=str, help='Password.',
    default='1234')
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['username'] = args.username
  variables['admin_password'] = args.password
  for filename in os.listdir('.'):
    default_path = os.path.join(filename, 'config.default.yml')
    if filename.startswith('utp_') and os.path.isdir(filename) and \
        os.path.isfile(default_path):
      with open(default_path, 'r+') as file:
        source = translate(file.read(), variables)
        file.seek(0)
        file.write(source)
        file.truncate()
        shutil.move(default_path, os.path.join(filename, 'config.yml'))

if __name__ == '__main__':
  main()
