import argparse
import beam
import datetime
import nexus
import sys
import yaml

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

def execute_report(account, start_date, end_date, fee_table, service_clients):
  definitions_client = service_clients.get_definitions_client()
  venues = definitions_client.load_venue_database()
  time_zones = definitions_client.load_time_zones()
  order_execution_client = service_clients.get_order_execution_client()
  order_queue = beam.Queue()
  nexus.query_daily_order_submissions(account, start_date, end_date, venues,
    time_zones, order_execution_client, order_queue)
  orders = []
  beam.flush(order_queue, orders)
  fee_state = nexus.ConsolidatedTmxFeeTable.State()
  for order in orders:
    if order.info.fields.security.country != nexus.default_countries.CA:
      continue
    if order.info.fields.destination == nexus.default_destinations.MOE:
      continue
    execution_reports = order.publisher.get_snapshot()
    for execution_report in execution_reports:
      calculated_fee = nexus.ExecutionReport(execution_report)
      calculated_fee.execution_fee = nexus.Money.ZERO
      calculated_fee.processing_fee = nexus.Money.ZERO
      calculated_fee.commission = nexus.Money.ZERO
      calculated_fee = nexus.calculate_fee(fee_table, fee_state, order,
        calculated_fee)
      if (calculated_fee.execution_fee, calculated_fee.processing_fee,
          calculated_fee.commission) != \
         (execution_report.execution_fee, execution_report.processing_fee,
          execution_report.commission):
        print('%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s' %
          (order.info.order_id, execution_report.sequence,
          order.info.fields.security.symbol, order.info.fields.security.venue,
          order.info.fields.side, execution_report.last_price,
          execution_report.last_quantity, execution_report.last_market,
          execution_report.liquidity_flag, execution_report.execution_fee,
          execution_report.processing_fee, execution_report.commission,
          calculated_fee.execution_fee, calculated_fee.processing_fee,
          calculated_fee.commission))
        sys.stdout.flush()

def load_fee_table(config, venues):
  fee_config = beam.load_yaml(config['fee_table'])
  return nexus.parse_consolidated_tmx_fee_table(fee_config, venues)

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
  service_clients = nexus.ServiceClients(username, password, address)
  venues = service_clients.get_definitions_client().load_venue_database()
  fee_table = load_fee_table(config, venues)
  for account in \
      service_clients.get_service_locator_client().load_all_accounts():
    execute_report(account, start_date, end_date, fee_table, service_clients)

if __name__ == '__main__':
  main()
