import argparse
import datetime
import functools
import multiprocessing
import sqlite3
import sys

import beam
import nexus
import pymysql
import pytz
import yaml

def parse_date(source):
  try:
    return datetime.datetime.strptime(source, '%Y-%m-%d %H:%M:%S')
  except ValueError:
    raise argparse.ArgumentTypeError("Not a valid date: '{0}'.".format(source))

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
  return beam.IpAddress(
    source[0:separator], int(source[separator + 1 :]))

def load_securities(source):
  securities = []
  cursor = source.cursor()
  query = 'SELECT DISTINCT `symbol`, `venue` FROM `bbo_quotes`'
  cursor.execute(query)
  for result in cursor.fetchall():
    securities.append(nexus.Security(result[0], nexus.Venue(result[1])))
  return securities

def load_venues(source):
  venues = []
  cursor = source.cursor()
  query = 'SELECT DISTINCT `venue` FROM `order_imbalances`'
  cursor.execute(query)
  for result in cursor.fetchall():
    venues.append(nexus.Venue(result[0]))
  return venues

def backup_security_info(source, destination):
  query = beam.PagedQuery()
  query.index = nexus.Region.GLOBAL
  query.snapshot_limit = beam.SnapshotLimit.UNLIMITED
  rows = source.load_security_info(query)
  for info in rows:
    destination.store(info)

def backup(index, start, end, loader, destination):
  if isinstance(index, nexus.Security):
    country = nexus.DEFAULT_VENUES.select(index.venue).country_code
    if country == nexus.default_countries.CA or \
        country == nexus.default_countries.US:
      timezone = pytz.timezone('US/Eastern')
    elif country == nexus.default_countries.AU:
      timezone = pytz.timezone('Australia/Sydney')
    elif country == nexus.default_countries.BR:
      timezone = pytz.timezone('America/Sao_Paulo')
    elif country == nexus.default_countries.CN:
      timezone = pytz.timezone('Asia/Shanghai')
    elif country == nexus.default_countries.HK:
      timezone = pytz.timezone('Asia/Hong_Kong')
    elif country == nexus.default_countries.JP:
      timezone = pytz.timezone('Asia/Tokyo')
  else:
    timezone = pytz.timezone('US/Eastern')
  localized_start = timezone.localize(start)
  localized_end = timezone.localize(end)
  utc_start = localized_start.astimezone(pytz.utc)
  utc_end = localized_end.astimezone(pytz.utc)
  query = beam.Query()
  query.index = index
  query.snapshot_limit = beam.SnapshotLimit.UNLIMITED
  query.set_range(utc_start, utc_end)
  values = loader(query)
  for i in range(len(values)):
    values[i] = beam.SequencedValue(
      beam.IndexedValue(values[i].value, index), values[i].sequence)
  destination.store(values)

def backup_spawn(index, start, end, source, destination):
  backup(index, start, end, source.load_bbo_quotes, destination)
  backup(index, start, end, source.load_time_and_sales, destination)
  backup(index, start, end, source.load_book_quotes, destination)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file.',
    default='config.yml')
  parser.add_argument(
    '-s', '--start', type=parse_date, help='Start time range.', required=True)
  parser.add_argument(
    '-e', '--end', type=parse_date, help='End time range.', required=True)
  parser.add_argument('-o', '--output', type=str, help='SQLite output file.')
  parser.add_argument('-i', '--input', type=str, help='SQLite input file.')
  parser.add_argument('-j', '--cores', type=int, help='Number of cores to use',
    required=False, default=(multiprocessing.cpu_count() - 1))
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
  if args.output and args.input:
    print('Can not use Sqlite as both the input and output.')
    exit(1)
  elif not args.output and not args.input:
    print('Sqlite path not specified.')
    exit(1)
  data_store_config = config['data_store']
  address = parse_ip_address(data_store_config['address'])
  username = data_store_config['username']
  password = data_store_config['password']
  schema = data_store_config['schema']
  if args.output is not None:
    sqlite_path = args.output
    connection = pymysql.connect(
      host=address.host, user=username, password=password, database=schema,
      port=address.port)
  else:
    sqlite_path = args.input
    connection = sqlite3.connect(sqlite_path)
  securities = load_securities(connection)
  venues = load_venues(connection)
  connection.close()
  mysql_data_store = nexus.MySqlHistoricalDataStore(
    address.host, address.port, username, password, schema)
  sqlite_data_store = nexus.SqliteHistoricalDataStore(sqlite_path)
  if args.output is not None:
    source = mysql_data_store
    destination = sqlite_data_store
  else:
    source = sqlite_data_store
    destination = mysql_data_store
  routines = beam.RoutineHandlerGroup()
  count = 0
  for security in securities:
    routines.spawn(functools.partial(backup_spawn, security, args.start,
      args.end, source, destination))
    count += 1
    if count % args.cores == 0:
      routines.wait()
  routines.wait()
  routines = beam.RoutineHandlerGroup()
  count = 0
  for venue in venues:
    routines.spawn(functools.partial(backup, venue, args.start, args.end,
      source.load_order_imbalances, destination))
    count += 1
    if count % args.cores == 0:
      routines.wait()
  routines.wait()
  backup_security_info(source, destination)

if __name__ == '__main__':
  main()
