import argparse
import datetime
import sys

import yaml

import beam
import nexus

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' % \
      (error.problem_mark.line, error.problem_mark.column, str(error.problem)))
  else:
    sys.stderr.write('Invalid YAML provided\n')

def parse_date(source):
  try:
    return datetime.datetime.strptime(source, '%Y-%m-%d %H:%M:%S')
  except ValueError:
    try:
      return datetime.datetime.strptime(source, '%Y-%m-%d')
    except ValueError:
      raise argparse.ArgumentTypeError(
        "Not a valid date: '{0}'.".format(source))

def parse_ip_address(source):
  separator = source.find(':')
  if separator == -1:
    return beam.network.IpAddress(source, 0)
  return beam.network.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-d', '--date', type=parse_date, help='Date',
    required=True)
  parser.add_argument('-m', '--destination', type=str, help='Destination',
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
  date = beam.time_service.to_utc_time(args.date)
  section = config['service_locator']
  address = parse_ip_address(section['address'])
  username = section['username']
  password = section['password']
  service_clients = nexus.ApplicationServiceClients(username, password, address)
  market_database = \
    service_clients.get_definitions_client().load_market_database()
  time_zone_database = \
    service_clients.get_definitions_client().load_time_zone_database()
  for account in \
      service_clients.get_service_locator_client().load_all_accounts():
    queue = beam.Queue()
    nexus.order_execution_service.query_daily_order_submissions(account, date,
      date, market_database, time_zone_database,
      service_clients.get_order_execution_client(), queue)
    orders = []
    beam.flush(queue, orders)
    for order in orders:
      if order.info.fields.destination != args.destination:
        continue
      execution_reports = order.get_publisher().get_snapshot()
      if not nexus.is_terminal(execution_reports[-1].status):
        cancel_report = \
          nexus.order_execution_service.ExecutionReport.build_updated_report(
          execution_reports[-1], nexus.OrderStatus.CANCELED,
          service_clients.get_time_client().get_time())
        service_clients.get_order_execution_client().update(order.info.order_id,
          cancel_report)

if __name__ == '__main__':
  main()
