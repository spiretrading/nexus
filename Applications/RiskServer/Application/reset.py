import argparse
import sys

import yaml

import aspen
import beam
import nexus

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
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file.',
    default='config.yml')
  parser.add_argument('-r', '--region', type=str, help='The region to reset.')
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
  section = config['service_locator']
  address = parse_ip_address(section['address'])
  username = section['username']
  password = section['password']
  service_clients = \
    nexus.ApplicationServiceClients(username, password, address)
  countries = service_clients.get_definitions_client().load_country_database()
  markets = service_clients.get_definitions_client().load_market_database()
  region = nexus.parse_country_code(args.region, countries)
  if region == nexus.CountryCode.NONE:
    region = nexus.parse_market_code(args.region, markets)
    if region != '':
      region = markets.from_code(region)
    else:
      region = nexus.parse_security(args.region, markets)
  service_clients.get_risk_client().reset(nexus.Region(region))

if __name__ == '__main__':
  main()
