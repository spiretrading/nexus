import argparse
import beam
import datetime
import nexus
import pymysql
import pytz
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
    return beam.network.IpAddress(source, 0)
  return beam.network.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def backup_bbo_quotes(security, start, end, source, destination):
  if security.country == nexus.default_countries.CA or \
      security.country == nexus.default_countries.US:
    timezone = pytz.timezone('US/Eastern')
  elif security.country == nexus.default_countries.AU:
    timezone = pytz.timezone('Australia/Sydney')
  localized_start = timezone.localize(start)
  localized_end = timezone.localize(end)
  utc_start = localized_start.astimezone(pytz.utc)
  utc_end = localized_end.astimezone(pytz.utc)
  query = beam.queries.Query()
  query.index = security
  query.snapshot_limit = beam.queries.SnapshotLimit.UNLIMITED
  query.set_range(utc_start, utc_end)
  values = source.load_bbo_quotes(query)
  for i in range(len(values)):
    print(values[i].sequence.ordinal)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2018 Eidolon Systems Ltd.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-s', '--start', type=parse_date, help='Start range',
    required=True)
  parser.add_argument('-e', '--end', type=parse_date, help='End range',
    required=True)
  parser.add_argument('-o', '--out', type=str, help='SQLite File',
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
  data_store_config = config['data_store']
  address = parse_ip_address(data_store_config['address'])
  username = data_store_config['username']
  password = data_store_config['password']
  schema = data_store_config['schema']
  mysql_data_store = nexus.market_data_service.MySqlHistoricalDataStore(
    address.host, address.port, username, password, schema)
  mysql_data_store.open()
  mysql_connection = pymysql.connect(address.host, username, password, schema,
    address.port)
  securities = []
  with mysql_connection.cursor() as cursor:
    query = 'SELECT DISTINCT `symbol`, `country` FROM `bbo_quotes`'
    cursor.execute(query)
    for result in cursor.fetchall():
      securities.append(nexus.Security(result[0], result[1]))
  markets = []
  with mysql_connection.cursor() as cursor:
    query = 'SELECT DISTINCT `market` FROM `order_imbalances`'
    cursor.execute(query)
    for result in cursor.fetchall():
      markets.append(result[0])
  sqlite_data_store = nexus.market_data_service.SqliteHistoricalDataStore(
    args.out)
  sqlite_data_store.open()
  for security in securities:
    backup_bbo_quotes(security, args.start, args.end, mysql_data_store,
      sqlite_data_store)

if __name__ == '__main__':
  main()
