import argparse
import os
import sys

def output(source_path, service_locator_address, local_interface, username,
    password):
  source = open(os.path.join(source_path, 'config.default.yml'), 'r').read()
  source = source.replace('$service_locator_address', service_locator_address)
  source = source.replace('$local_interface', local_interface)
  source = source.replace('$username', '"%s"' % username)
  source = source.replace('$password', '"%s"' % password)
  destination = open(os.path.join(source_path, 'config.yml'), 'w')
  destination.write(source)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2009 Eidolon Systems Ltd.')
  parser.add_argument('-s', '--service', type=str,
    help='Service Locator Address', required=True)
  parser.add_argument('-l', '--local', type=str, help='Local Interface',
    required=True)
  parser.add_argument('-u', '--username', type=str, help='Username',
    required=False, default='market_data_feed')
  parser.add_argument('-p', '--password', type=str, help='Password',
    required=True)
  args = parser.parse_args()
  if args.service.find(':') == -1:
    args.service = '%s:20000' % args.service
  for filename in os.listdir('.'):
    if filename.startswith('cse_') and os.path.isdir(filename):
      output(filename, args.service, args.local, args.username, args.password)

if __name__ == '__main__':
  main()
