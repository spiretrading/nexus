import argparse
import beam
import mysql.connector
import nexus
import sys
import yaml

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' % \
      (error.problem_mark.line, error.problem_mark.column, str(error.problem)))
  else:
    sys.stderr.write('Invalid YAML provided\n')

def parse_ip_address(source):
  separator = source.find(':')
  if separator == -1:
    return beam.network.IpAddress(source, 0)
  return beam.network.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-c', '--config', type = str, help = 'Configuration file',
    default = 'config.yml')
  parser.add_argument('-r', '--report', type = str, help = 'Report file',
    required = True)
  args = parser.parse_args()
  try:
    stream = open(args.config, 'r').read()
    config = yaml.load(stream, yaml.SafeLoader)
  except IOError:
    sys.stderr.write('%s not found\n' % args.config)
    exit(1)
  except yaml.YAMLError as e:
    report_yaml_error(e)
    exit(1)
  service_locator_config = config['service_locator']
  address = parse_ip_address(service_locator_config['address'])
  username = service_locator_config['username']
  password = service_locator_config['password']
  data_store_config = config['data_store']
  mysql_address = parse_ip_address(data_store_config['address'])
  mysql_username = data_store_config['username']
  mysql_password = data_store_config['password']
  schema = data_store_config['schema']
  connection = mysql.connector.connect(host = mysql_address.host,
    database = schema, user = mysql_username, password = mysql_password)
  for line in open(args.report, 'r')
    print line

if __name__ == '__main__':
  main()
