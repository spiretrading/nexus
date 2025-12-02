import argparse
import beam
import datetime
import nexus
import sys
import yaml

def execute_report(start_date, end_date, security, venue, account,
    venues, time_zones, service_locator_client, order_execution_client):
  orders = []
  activity_log = []
  if account is not None:
    order_queue = beam.Queue()
    nexus.query_daily_order_submissions(account, start_date, end_date, venues,
      time_zones, order_execution_client, order_queue)
    account_orders = []
    beam.flush(order_queue, account_orders)
    for order in account_orders:
      if security is not None and security != order.info.fields.security:
        continue
      if venue is not None and venue != order.info.fields.security.venue:
        continue
      orders.append(order)
      execution_reports = order.get_publisher().get_snapshot()
      for execution_report in execution_reports:
        activity_log.append((order.info.fields, execution_report))
  else:
    accounts = service_locator_client.load_all_accounts()
    for account in accounts:
      (account_orders, account_log) = execute_report(start_date, end_date,
        security, venue, account, venues, time_zones, service_locator_client,
        order_execution_client)
      orders += account_orders
      activity_log += account_log
    orders.sort(key = lambda value: value.info.timestamp)
    activity_log.sort(key = lambda value: value[1].timestamp)
  return (orders, activity_log)

def output_order_log(orders):
  for order in orders:
    print('%s,%s,%s,%s,%s,%s,%s,%s,%s,%s' % (order.info.timestamp,
      order.info.fields.account.name, order.info.order_id,
      order.info.fields.security.symbol, order.info.fields.type,
      order.info.fields.side, order.info.fields.destination,
      order.info.fields.quantity, order.info.fields.price,
      order.info.fields.time_in_force.type))

def output_activity_log(activity_log):
  for report in activity_log:
    print('%s,%s,%s,%s,%s,%s,%s,%s,%s' % (report[1].timestamp, report[1].id,
      report[0].side, report[0].security.symbol, report[1].status,
      report[1].last_quantity, report[1].last_price, report[1].last_market,
      report[1].liquidity_flag))

def parse_date(source):
  try:
    return datetime.datetime.strptime(source, '%Y-%m-%d %H:%M:%S')
  except ValueError:
    raise argparse.ArgumentTypeError(
      "Not a valid date: '{0}'.".format(source))

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
  parser.add_argument('-s', '--start', type=parse_date, help='Start range',
    required=True)
  parser.add_argument('-e', '--end', type=parse_date, help='End range',
    required=True)
  parser.add_argument('-v', '--venue', type=str, help='Venue')
  parser.add_argument('-t', '--symbol', type=str, help='Ticker symbol')
  parser.add_argument('-a', '--account', type=str, help='Account')
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
  address = parse_ip_address(config['service_locator'])
  username = config['username']
  password = config['password']
  start_date = beam.to_utc_time(args.start)
  end_date = beam.to_utc_time(args.end)
  service_locator_client = beam.ApplicationServiceLocatorClient(
    username, password, address)
  definitions_client = nexus.ApplicationDefinitionsClient(
    service_locator_client)
  order_execution_client = nexus.ApplicationOrderExecutionClient(
    service_locator_client)
  venues = definitions_client.load_venue_database()
  time_zones = definitions_client.load_time_zones()
  if args.venue is not None:
    venue = nexus.parse_venue(args.venue, venues)
  else:
    venue = None
  if args.symbol is not None:
    security = nexus.parse_security(args.symbol, venues)
  else:
    security = None
  if args.account is not None:
    account = service_locator_client.find_account(args.account)
  else:
    account = None
  (orders, activity_log) = execute_report(start_date, end_date, security,
    venue, account, venues, time_zones, service_locator_client,
    order_execution_client)
  output_order_log(orders)
  output_activity_log(activity_log)

if __name__ == '__main__':
  main()
