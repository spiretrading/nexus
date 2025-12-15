import argparse
import datetime
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


def parse_side(source):
  if source.lower() == 'bid':
    return nexus.Side.BID
  elif source.lower() == 'ask':
    return nexus.Side.ASK
  raise RuntimeError('Invalid side provided.')


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-m', '--market', type=str,
    help='The disseminating market.', required=True)
  parser.add_argument('-s', '--symbol', type=str, help='Ticker symbol.',
    required=True)
  parser.add_argument('-t', '--side', type=parse_side, help='Imbalance side.',
    required=True)
  parser.add_argument('-q', '--quantity', type=nexus.Quantity.from_value,
    help='Imbalance quantity.', required=True)
  parser.add_argument('-p', '--reference_price', type=nexus.Money.from_value,
    help='Reference price.')
  parser.add_argument('-d', '--timestamp', type=datetime.datetime,
    help='Timestamp in UTC.')
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
  service_clients = nexus.ServiceClients(username, password, address)
  markets = service_clients.get_definitions_client().load_market_database()
  market = nexus.parse_market_code(args.market, markets)
  feed_client = nexus.ApplicationMarketDataFeedClient(
    service_clients.get_service_locator_client())
  timestamp = args.timestamp if args.timestamp is not None else \
    datetime.datetime.utcnow()
  order_imbalance = beam.IndexedValue(nexus.OrderImbalance(
    nexus.parse_security(args.symbol), args.side, args.quantity,
    args.reference_price, timestamp), market)
  feed_client.publish(order_imbalance)

if __name__ == '__main__':
  main()
