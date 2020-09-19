import argparse
import datetime
import sys
import urllib.request

import xlrd
import yaml

import beam
import nexus

URL = 'https://www.hkex.com.hk/eng/services/trading/securities/securitieslists/ListOfSecurities.xlsx'

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' % \
      (error.problem_mark.line, error.problem_mark.column, str(error.problem)))
  else:
    sys.stderr.write('Invalid YAML provided\n')

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
  parser.add_argument('-d', '--dry', action='store_true', help='Dry run.')
  args = parser.parse_args()
  if not args.dry:
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
    service_locator_client = \
      beam.service_locator.ApplicationServiceLocatorClient(username, password,
      address)
    feed_client = nexus.market_data_service.ApplicationMarketDataFeedClient(
      service_locator_client, nexus.default_countries.HK)
  with urllib.request.urlopen(URL) as response, open('hkex.xlsx', 'wb') as \
      destination:
    destination.write(response.read())
  book = xlrd.open_workbook('hkex.xlsx')
  sheet = book.sheet_by_index(0)
  STOCK_COLUMN = None
  NAME_COLUMN = None
  BOARD_LOT_COLUMN = None
  for c in range(0, sheet.ncols):
    if sheet.row_values(2)[c] == 'Stock Code':
      STOCK_COLUMN = c
    elif sheet.row_values(2)[c] == 'Name of Securities':
      NAME_COLUMN = c
    elif sheet.row_values(2)[c] == 'Board Lot':
      BOARD_LOT_COLUMN = c
  if STOCK_COLUMN is None:
    print('Stock code column not found.')
    sys.exit(-1)
  elif NAME_COLUMN is None:
    print('Name column not found.')
    sys.exit(-1)
  for i in range(3, sheet.nrows):
    code = int(sheet.row_values(i)[STOCK_COLUMN])
    security_info = nexus.SecurityInfo()
    security_info.security = nexus.parse_security('%s.HKEX' % code)
    security_info.name = sheet.row_values(i)[NAME_COLUMN]
    security_info.board_lot = nexus.Quantity.from_value(
      sheet.row_values(i)[BOARD_LOT_COLUMN].replace(',', ''))
    if args.dry:
      print(security_info)
    else:
      feed_client.add(security_info)

if __name__ == '__main__':
  main()
