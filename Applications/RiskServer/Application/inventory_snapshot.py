import argparse
import sys

import beam
import nexus
import yaml

CHUNK_SIZE = 10000

def load_order_records(account, data_store):
  sequence = beam.Sequence.FIRST
  while True:
    query = beam.Query()
    query.index = account
    query.set_range(sequence, beam.Sequence.LAST)
    query.set_snapshot_limit(beam.SnapshotLimit.Type.HEAD, CHUNK_SIZE)
    records = data_store.load_order_records(query)
    for record in records:
      yield record
    if len(records) < CHUNK_SIZE:
      return
    sequence = beam.increment(records[-1].sequence)

def recalculate_snapshot(account, data_store):
  model = nexus.InventorySnapshotModel()
  count = 0
  for record in load_order_records(account, data_store):
    model.add(record.sequence, nexus.PrimitiveOrder(record.value))
    count += 1
  return model.make_snapshot(), count

def update_snapshot(account, source, order_execution_data_store,
    risk_data_store):
  snapshot, count = recalculate_snapshot(account, source)
  order_execution_data_store.store(account, snapshot)
  risk_data_store.store(account, snapshot)
  print(f'{account.name}: {count} orders, {len(snapshot.inventories)} '
    f'inventories, {len(snapshot.excluded_orders)} excluded orders, sequence '
    f'{snapshot.sequence.ordinal}')

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
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-o', '--output', type=str, help='SQLite output file',
    default='inventory_snapshots.db')
  selection = parser.add_mutually_exclusive_group(required=True)
  selection.add_argument('-a', '--account', type=str,
    help='Account to recalculate.')
  selection.add_argument('--all', action='store_true',
    help='Recalculate every account.')
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
  nexus.load_definitions(service_clients.definitions_client)
  if args.all:
    accounts = service_clients.service_locator_client.load_all_accounts()
  else:
    account = service_clients.service_locator_client.find_account(args.account)
    if not account:
      print(f'Account {args.account} not found.')
      return
    accounts = [account]
  section = config['data_store']
  address = parse_ip_address(section['address'])
  source = nexus.MySqlOrderExecutionDataStore(address.host, address.port,
    section['username'], section['password'], section['schema'])
  order_execution_data_store = nexus.SqliteOrderExecutionDataStore(args.output)
  risk_data_store = nexus.SqliteRiskDataStore(args.output)
  for account in accounts:
    update_snapshot(
      account, source, order_execution_data_store, risk_data_store)
  risk_data_store.close()
  order_execution_data_store.close()
  source.close()

if __name__ == '__main__':
  main()
