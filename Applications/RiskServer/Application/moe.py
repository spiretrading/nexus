import argparse
from dataclasses import dataclass
from enum import Enum
import sys

import beam
import csv
import nexus
import yaml

@dataclass
class Entry:
  account: str
  position: nexus.Position

def parse_positions(file_path, currencies):
  positions = []
  with open(file_path, newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
      position = nexus.Position()
      position.key = nexus.Position.Key(nexus.parse_security(row['Security']),
        currencies.from_code(row['Currency']).id)
      position.quantity = nexus.parse_quantity(row['Open Quantity'])
      position.cost_basis = nexus.parse_money(row['Cost Basis'])
      if row['Side'] == 'Short':
        position.quantity = -position.quantity
        position.cost_basis = -position.cost_basis
      positions.append(Entry(row['Account'], position))
  return positions

class Mode(Enum):
  MOE_IN = 'in'
  MOE_OUT = 'out'

def moe(service_clients, position, destination, mode):
  directory_entry = service_clients.service_locator_client.find_account(
    position.account)
  if directory_entry is None:
    return
  side = nexus.side(position.position)
  if mode == Mode.MOE_OUT:
    side = nexus.opposite(side)
  price = abs(nexus.average_price(position.position))
  fields = nexus.make_limit_order_fields(
    directory_entry, position.position.key.index,
    position.position.key.currency, side, destination,
    abs(position.position.quantity), price)
  service_clients.order_execution_client.submit(fields)

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
    description='v1.0 Copyright (C) 2025 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-a', '--account', type=str, help='Account name.')
  parser.add_argument('-r', '--region', type=str, help='The region to MOE.')
  parser.add_argument('-p', '--positions', type=str,
    help='Path to positions file', required=True)
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument('--open', dest='mode', action='store_const',
    const=Mode.MOE_IN, help='Flag indicating to open the positions')
  group.add_argument('--close', dest='mode', action='store_const',
    const=Mode.MOE_OUT, help='Flag indicating to close the positions')
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
  countries = service_clients.definitions_client.load_country_database()
  venues = service_clients.definitions_client.load_venue_database()
  if args.region is None:
    region = nexus.Region.GLOBAL
  else:
    region = nexus.parse_country_code(args.region, countries)
    if region == nexus.CountryCode.NONE:
      region = nexus.parse_venue(args.region, venues)
      if region:
        region = venues.select(region).venue
      else:
        region = nexus.parse_security(args.region, venues)
    region = nexus.Region(region)
  positions = parse_positions(args.positions,
    service_clients.definitions_client.load_currency_database())
  destinations = \
    service_clients.definitions_client.load_destination_database()
  destination = destinations.manual_order_entry_destination.id
  for position in positions:
    if region.contains(nexus.Region(position.position.key.index)):
      if args.account:
        if position.account == args.account:
          moe(service_clients, position, destination, args.mode)
      else:
        moe(service_clients, position, destination, args.mode)

if __name__ == '__main__':
  main()
