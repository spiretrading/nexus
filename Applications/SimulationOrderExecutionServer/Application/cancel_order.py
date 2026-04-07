import argparse
import datetime
import functools
import os
import sys

import beam
import nexus
import yaml

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
    return beam.IpAddress(source, 0)
  return beam.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def parse_region(service_clients, region):
  if region == '*':
    return nexus.Region.GLOBAL
  venue_database = \
    service_clients.definitions_client.load_venue_database()
  countries = service_clients.definitions_client.load_country_database()
  region = nexus.parse_country_code(region, countries)
  if region == nexus.CountryCode.NONE:
    region = nexus.parse_venue(region)
    if region:
      region = venue_database.from_code(region)
    else:
      region = nexus.parse_security(region)
  return nexus.Region(region)

def cancel_order(service_clients, order, message):
  execution_reports = order.publisher.get_snapshot()
  if len(execution_reports) == 0:
    pending_new_report = nexus.ExecutionReport(
      order.info.id, service_clients.time_client.get_time())
    service_clients.order_execution_client.update(
      order.info.id, pending_new_report)
    execution_reports = [pending_new_report]
  if not nexus.is_terminal(execution_reports[-1].status):
    cancel_report = nexus.make_update(
      execution_reports[-1], nexus.OrderStatus.CANCELED,
      service_clients.time_client.get_time())
    cancel_report.text = message
    service_clients.order_execution_client.update(
      order.info.id, cancel_report)

def cancel_order_by_id(service_clients, order_id, message):
  order = service_clients.order_execution_client.load_order(order_id)
  cancel_order(service_clients, order, message)

def cancel_account(service_clients, account, region, begin, end, message):
  venue_database = \
    service_clients.definitions_client.load_venue_database()
  time_zone_database = \
    service_clients.definitions_client.load_time_zone_database()
  queue = beam.Queue()
  nexus.query_daily_order_submissions(account, begin, end, venue_database,
    time_zone_database, service_clients.order_execution_client, queue)
  orders = []
  beam.flush(queue, orders)
  for order in orders:
    if nexus.Region(order.info.fields.security) <= region:
      cancel_order(service_clients, order, message)

def cancel_region(service_clients, region, begin, end, message):
  count = 0
  routines = beam.RoutineHandlerGroup()
  for account in \
      service_clients[0].service_locator_client.load_all_accounts():
    if count == len(service_clients):
      count = 0
      routines.wait()
    routines.spawn(functools.partial(cancel_account, service_clients[count],
      account, region, begin, end, message))
    count += 1
  routines.wait()

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-j', '--connections', type=int,
    help='Number of connections', default=(os.cpu_count() // 2))
  parser.add_argument('-o', '--order', type=int, help='Order ID.')
  parser.add_argument('-a', '--account', type=str, help='Account name.')
  parser.add_argument('-r', '--region', type=str, help='The region to cancel.')
  parser.add_argument('-b', '--begin', type=parse_date, help='Start date.')
  parser.add_argument('-e', '--end', type=parse_date, help='End date.')
  parser.add_argument('-m', '--message', type=str, help='Cancel message.',
    default='Session terminated.')
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
  service_clients = []
  for i in range(args.connections):
    service_clients.append(nexus.ServiceClients(username, password, address))
  if args.order:
    cancel_order_by_id(service_clients[0], args.order, args.message)
  elif args.account:
    if args.region:
      region = parse_region(service_clients[0], args.region)
    else:
      region = nexus.Region.GLOBAL
    account = service_clients[0].service_locator_client.find_account(
      args.account)
    if not account:
      print('Account %s not found.' % args.account)
      return
    cancel_account(service_clients[0], account, region, args.begin, args.end,
      args.message)
  elif args.region:
    cancel_region(service_clients,
      parse_region(service_clients[0], args.region),
      args.begin, args.end, args.message)
  else:
    print('The --account or --region argument is required.')

if __name__ == '__main__':
  main()
