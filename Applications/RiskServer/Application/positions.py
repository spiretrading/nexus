import argparse
import csv
import sys

import beam
import nexus
import yaml

def report_positions(service_clients, account, venues, currencies, writer):
  snapshot = service_clients.get_risk_client().load_inventory_snapshot(account)
  portfolio, sequence, excluded_orders = nexus.make_portfolio(
    snapshot, account, venues, service_clients.get_order_execution_client())
  for order in excluded_orders:
    execution_reports = order.get_publisher().get_snapshot()
    if execution_reports is not None:
      for execution_report in execution_reports:
        portfolio.update(order.info.fields, execution_report)
  for security in portfolio.security_entries:
    currency = venues.from_code(security.venue).currency
    inventory = portfolio.bookkeeper.get_inventory(security, currency)
    position = inventory.position
    if nexus.side(position) == nexus.Side.BID:
      side_code = 'Long'
    else:
      side_code = 'Short'
    writer.writerow([account.name, str(security),
      currencies.from_id(currency).code, side_code, position.quantity,
      position.cost_basis])

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' %
      (error.problem_mark.line, error.problem_mark.column, error.problem))
  else:
    sys.stderr.write('Invalid YAML provided\n')

def parse_ip_address(source):
  separator = source.find(':')
  if separator == -1:
    return beam.IpAddress(source, 0)
  return beam.IpAddress(source[:separator], int(source[separator + 1:]))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2025 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-a', '--account', type=str, help='Account name.')
  args = parser.parse_args()
  try:
    stream = open(args.config, 'r').read()
    config = yaml.load(stream, yaml.SafeLoader)
  except IOError:
    sys.stderr.write(f'{args.config} not found\n')
    exit(1)
  except yaml.YAMLError as e:
    report_yaml_error(e)
    exit(1)
  section = config['service_locator']
  address = parse_ip_address(section['address'])
  username = section['username']
  password = section['password']
  service_clients = nexus.ServiceClients(username, password, address)
  csv_writer = csv.writer(sys.stdout, quoting=csv.QUOTE_NONNUMERIC)
  csv_writer.writerow(["Account", "Security", "Currency", "Side",
    "Open Quantity", "Cost Basis"])
  venues = service_clients.get_definitions_client().load_venue_database()
  currencies = service_clients.get_definitions_client().load_currency_database()
  if args.account:
    account = service_clients.get_service_locator_client().find_account(
      args.account)
    if not account:
      print(f'Account {args.account} not found.')
      return
    report_positions(service_clients, account, venues, currencies, csv_writer)
  else:
    for account in \
        service_clients.get_service_locator_client().load_all_accounts():
      report_positions(service_clients, account, venues, currencies,
        csv_writer)

if __name__ == '__main__':
  main()
