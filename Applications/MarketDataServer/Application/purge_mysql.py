import argparse
import copy
import datetime
import multiprocessing
import sys
import threading

import beam
import nexus
import pymysql
import pytz
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

def purge_security(security, start, end, connection, table):
  if security.country == nexus.default_countries.CA or \
      security.country == nexus.default_countries.US:
    timezone = pytz.timezone('US/Eastern')
  elif security.country == nexus.default_countries.AU:
    timezone = pytz.timezone('Australia/Sydney')
  localized_start = timezone.localize(start)
  localized_end = timezone.localize(end)
  utc_start = localized_start.astimezone(pytz.utc)
  utc_end = localized_end.astimezone(pytz.utc)
  start_timestamp = beam.to_sql_timestamp(utc_start)
  end_timestamp = beam.to_sql_timestamp(utc_end)
  with connection.cursor() as cursor:
    query = 'DELETE FROM %s WHERE symbol = "%s" AND country = "%s" AND ' \
      'timestamp >= %s and timestamp <= %s' % \
      (table, security.symbol, security.country, start_timestamp, end_timestamp)
    cursor.execute(query)
  connection.commit()

def purge_market(market, start, end, connection, table):
  timezone = pytz.timezone('US/Eastern')
  localized_start = timezone.localize(start)
  localized_end = timezone.localize(end)
  utc_start = localized_start.astimezone(pytz.utc)
  utc_end = localized_end.astimezone(pytz.utc)
  start_timestamp = beam.to_sql_timestamp(utc_start)
  end_timestamp = beam.to_sql_timestamp(utc_end)
  with connection.cursor() as cursor:
    query = 'DELETE FROM %s WHERE market = "%s" AND ' \
      'timestamp >= %s and timestamp <= %s' % \
      (table, market, start_timestamp, end_timestamp)
    cursor.execute(query)
  connection.commit()

def purge_all_security(security, start, end, connection):
  purge_security(security, start, end, connection, 'bbo_quotes')
  purge_security(security, start, end, connection, 'book_quotes')
  purge_security(security, start, end, connection, 'time_and_sales')
  purge_security(security, start, end, connection, 'market_quotes')

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-s', '--start', type=parse_date, help='Start range',
    required=True)
  parser.add_argument('-e', '--end', type=parse_date, help='End range',
    required=True)
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
  data_store_config = config['data_store']
  address = parse_ip_address(data_store_config['address'])
  username = data_store_config['username']
  password = data_store_config['password']
  schema = data_store_config['schema']
  mysql_connection = pymysql.connect(host=address.host, user=username,
    password=password, database=schema, port=address.port)
  securities = []
  with mysql_connection.cursor() as cursor:
    query = 'SELECT DISTINCT `symbol`, `country` FROM `bbo_quotes`'
    cursor.execute(query)
    for result in cursor.fetchall():
      securities.append(nexus.Security(result[0],
        nexus.CountryCode(int(result[1]))))
  markets = []
  with mysql_connection.cursor() as cursor:
    query = 'SELECT DISTINCT `market` FROM `order_imbalances`'
    cursor.execute(query)
    for result in cursor.fetchall():
      markets.append(result[0])
  threads = []
  for security in securities:
    if len(threads) == args.cores:
      for thread in threads:
        thread.join()
      threads = []
    connection = pymysql.connect(host=address.host, user=username,
      password=password, database=schema, port=address.port)
    thread = threading.Thread(target = purge_all_security,
      args = (security, args.start, args.end, connection))
    threads.append(thread)
    thread.start()
  for thread in threads:
    thread.join()
  threads = []
  for market in markets:
    if len(threads) == args.cores:
      for thread in threads:
        thread.join()
      threads = []
    connection = pymysql.connect(host=address.host, user=username,
      password=password, database=schema, port=address.port)
    thread = threading.Thread(target = purge_market,
      args = (market, args.start, args.end, connection, 'order_imbalances'))
    threads.append(thread)
    thread.start()
  for thread in threads:
    thread.join()

if __name__ == '__main__':
  main()
