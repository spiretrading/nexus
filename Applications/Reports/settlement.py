import argparse
import beam
import datetime
import nexus
import sys
import yaml

from collections import Counter
from collections import defaultdict

def execute_report(start_date, end_date, currencies, venues, time_zones,
    service_locator_client, administration_client, order_execution_client):
  trading_group_entries = administration_client.load_managed_trading_groups(
    service_locator_client.account)
  for trading_group_entry in trading_group_entries:
    print(trading_group_entry.name)
    group_profit_and_loss = defaultdict(lambda: nexus.Money.ZERO)
    trading_group = administration_client.load_trading_group(
      trading_group_entry)
    for trader in trading_group.traders:
      account_portfolio = nexus.TrueAveragePortfolio(venues)
      account_volumes = Counter()
      order_queue = beam.Queue()
      nexus.query_daily_order_submissions(trader, start_date, end_date, venues,
        time_zones, order_execution_client, order_queue)
      orders = []
      beam.flush(order_queue, orders)
      for order in orders:
        execution_reports = order.get_publisher().get_snapshot()
        for execution_report in execution_reports:
          account_portfolio.update(order.info.fields, execution_report)
          account_volumes[order.info.fields.currency.value] += \
            execution_report.last_quantity
      if sum(account_volumes.itervalues()) > 0:
        print('\t%s' % trader.name)
        for currency_value in account_volumes.iterkeys():
          currency = nexus.CurrencyId(currency_value)
          account_totals = account_portfolio.bookkeeper.get_total(currency)
          net_profit_and_loss = nexus.get_realized_profit_and_loss(
            account_totals)
          group_profit_and_loss[currency.value] += net_profit_and_loss
          print('\t\tCurrency: %s' % currencies.from_id(currency).code)
          print('\t\t\tVolume: %s' % account_volumes[currency.value])
          print('\t\t\tP/L: %s\n' % net_profit_and_loss)
    if len(group_profit_and_loss) > 0:
      print('\tTotals')
      for currency_value in group_profit_and_loss.iterkeys():
        currency = nexus.CurrencyId(currency_value)
        print('\t\t%s: %s' % (currencies.from_id(currency).code,
          group_profit_and_loss[currency.value]))
      print('')

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
  administration_client = nexus.ApplicationAdministrationClient(
    service_locator_client)
  definitions_client = nexus.ApplicationDefinitionsClient(
    service_locator_client)
  order_execution_client = nexus.ApplicationOrderExecutionClient(
    service_locator_client)
  currencies = definitions_client.load_currencies()
  venues = definitions_client.load_venues()
  time_zones = definitions_client.load_time_zones()
  execute_report(start_date, end_date, currencies, venues, time_zones,
    service_locator_client, administration_client, order_execution_client)

if __name__ == '__main__':
  main()
