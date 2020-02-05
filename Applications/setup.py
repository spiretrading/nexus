import argparse
import os
import subprocess

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    required=False)
  parser.add_argument('-w', '--world', type=str, help='World interface.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=False)
  args = parser.parse_args()
  applications = [d for d in os.listdir('./') if os.path.isdir(d)]
  if args.local is None:
    local_interface = call(
      'echo -n `ifconfig -a | '
      'egrep -o "([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" | '
      'head -1`')[0].strip()
  else:
    local_interface = args.local
  if args.world is None:
    global_address = local_interface
  else:
    global_address = args.world
  if args.password is None:
    admin_password = '1234'
  else:
    admin_password = args.password
  for application in applications:
    application_directory = os.path.join('./', application)
    files = [f for f in os.listdir(application_directory) if
      os.path.isfile(os.path.join(application_directory, f)) and
      f.endswith('.default.yml')]
    for file in files:
      file_path = os.path.join(application_directory, file)
      destination_path = file_path.replace('.default.yml', '.yml')
      call('mv %s %s' % (file_path, destination_path))
      call('sed -i "s:local_interface:%s:g" %s' %
        (local_interface, destination_path))
      call('sed -i "s:global_address:%s:g" %s' %
        (global_address, destination_path))
      call('sed -i "s:admin_password:\\"%s\\":g" %s' %
        (admin_password, destination_path))

if __name__ == '__main__':
  main()
