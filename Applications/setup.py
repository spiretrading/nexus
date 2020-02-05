import argparse
import os
import shutil
import subprocess

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode(
    'utf-8')

def translate(source, variables):
  for key in variables.keys():
    source = source.replace('$' + key, '"%s"' % variables[key])
  return source

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    required=False)
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
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
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = \
    call('hostname -I').strip() if args.local is None else args.local
  variables['global_address'] = \
    variables['local_interface'] if args.world is None else args.world
  variables['admin_password'] = \
    '1234' if args.password is None else args.password
  variables['mysql_address'] = ('%s:3306' % variables['local_interface']) if \
    args.mysql_address is None else args.mysql_address
  variables['mysql_username'] = \
    'spireadmin' if args.mysql_username is None else args.mysql_username
  variables['mysql_password'] = \
    variables['admin_password'] if args.mysql_password is None else \
    args.mysql_password
  variables['mysql_schema'] = 'spire' if args.mysql_schema is None else \
    args.mysql_schema
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  applications = [d for d in os.listdir('./') if os.path.isdir(d)]
  for application in applications:
    application_directory = os.path.join('.', application)
    files = [f for f in os.listdir(application_directory) if
      os.path.isfile(os.path.join(application_directory, f)) and
      f.endswith('.default.yml')]
    for file in files:
      file_path = os.path.join(application_directory, file)
      destination_path = file_path.replace('.default.yml', '.yml')
      shutil.move(file_path, destination_path)
      with open(destination_path, 'r+') as file:
        source = translate(file.read(), variables)
        file.seek(0)
        file.write(source)
        file.truncate()

if __name__ == '__main__':
  main()
