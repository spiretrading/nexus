import argparse
import yaml

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
    return beam.IpAddress(source, 0)
  return beam.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-s', '--symbol', type=str, help='Ticker symbol.',
    required=True)
  parser.add_argument('-n', '--name', type=str, help='Display name.',
    required=True)
  parser.add_argument('-t', '--sector', type=str, help='Sector.', default='')
  parser.add_argument('-b', '--board_lot', type=str, help='Board lot.',
    required=True)
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
  service_locator_client = \
    beam.ApplicationServiceLocatorClient(username, password, address)
  feed_client = nexus.ApplicationMarketDataFeedClient(service_locator_client)
  security_info = nexus.SecurityInfo()
  security_info.security = nexus.parse_security('%s' % args.symbol)
  security_info.name = args.name
  security_info.sector = args.sector
  security_info.board_lot = nexus.parse_quantity(args.board_lot)
  feed_client.add(security_info)

if __name__ == '__main__':
  main()
