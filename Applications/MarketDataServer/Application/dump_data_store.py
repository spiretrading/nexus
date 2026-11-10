import argparse
import datetime
import functools
import multiprocessing
import sqlite3
import sys
import threading
import traceback
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing

import beam
import nexus
import pymysql
import pytz
import yaml

BATCH_SIZE = 100000
FLOAT_COLUMNS = frozenset(('ask_price', 'ask_size', 'bid_price', 'bid_size',
  'board_lot', 'price', 'size'))
TABLES = ('bbo_quotes', 'time_and_sales', 'book_quotes', 'order_imbalances',
  'ticker_info')

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

def quote_identifier(name):
  return '`' + name.replace('`', '``') + '`'

def get_placeholder(connection):
  if isinstance(connection, sqlite3.Connection):
    return '?'
  return '%s'

def open_cursor(connection):
  if isinstance(connection, sqlite3.Connection):
    return connection.cursor()
  return connection.cursor(pymysql.cursors.SSCursor)

def load_tickers(source):
  with closing(source.cursor()) as cursor:
    cursor.execute('SELECT DISTINCT `symbol`, `venue` FROM `bbo_quotes`')
    return [nexus.Ticker(symbol, nexus.Venue(venue))
      for symbol, venue in cursor]

def load_venues(source):
  with closing(source.cursor()) as cursor:
    cursor.execute('SELECT DISTINCT `venue` FROM `order_imbalances`')
    return [nexus.Venue(row[0]) for row in cursor]

def load_columns(connection):
  columns = {}
  with closing(connection.cursor()) as cursor:
    for table in TABLES:
      cursor.execute('PRAGMA table_info(' + quote_identifier(table) + ')')
      columns[table] = tuple(row[1] for row in cursor)
  return columns

def get_select_columns(connection, columns):
  names = []
  for name in columns:
    column = quote_identifier(name)
    if isinstance(connection, sqlite3.Connection) and name in FLOAT_COLUMNS:
      column = 'CAST(' + column + ' AS TEXT)'
    names.append(column)
  return ', '.join(names)

def format_rows(columns, rows):
  """Yields rows with Viper's six-decimal floating-point SQL representation."""
  indexes = [i for i, name in enumerate(columns) if name in FLOAT_COLUMNS]
  for row in rows:
    values = list(row)
    for i in indexes:
      values[i] = format(float(values[i]), '.6f')
    yield values

def get_range(index, start, end):
  if isinstance(index, nexus.Ticker):
    country = nexus.VENUES.select(index.venue).country_code
    if country == nexus.countries.CA or country == nexus.countries.US:
      timezone = pytz.timezone('US/Eastern')
    elif country == nexus.countries.AU:
      timezone = pytz.timezone('Australia/Sydney')
    elif country == nexus.countries.BR:
      timezone = pytz.timezone('America/Sao_Paulo')
    elif country == nexus.countries.CN:
      timezone = pytz.timezone('Asia/Shanghai')
    elif country == nexus.countries.HK:
      timezone = pytz.timezone('Asia/Hong_Kong')
    elif country == nexus.countries.JP:
      timezone = pytz.timezone('Asia/Tokyo')
  else:
    timezone = pytz.timezone('US/Eastern')
  epoch = datetime.datetime(1970, 1, 1, tzinfo=pytz.utc)
  start = timezone.localize(start).astimezone(pytz.utc) - epoch
  end = timezone.localize(end).astimezone(pytz.utc) - epoch
  return (start // datetime.timedelta(milliseconds=1),
    end // datetime.timedelta(milliseconds=1))

class Writer:
  def __init__(self, connection, columns):
    self.connection = connection
    self.columns = columns
    self.lock = threading.Lock()
    self.statements = {}
    placeholder = get_placeholder(connection)
    for table in TABLES:
      names = ', '.join(quote_identifier(name) for name in columns[table])
      parameters = ', '.join(placeholder for name in columns[table])
      statement = 'INSERT INTO ' + quote_identifier(table) + \
        ' (' + names + ') VALUES (' + parameters + ')'
      if table == 'ticker_info':
        updates = []
        for name in columns[table]:
          if name not in ('symbol', 'venue'):
            column = quote_identifier(name)
            if isinstance(connection, sqlite3.Connection):
              updates.append(column + ' = excluded.' + column)
            else:
              updates.append(column + ' = VALUES(' + column + ')')
        if isinstance(connection, sqlite3.Connection):
          statement += ' ON CONFLICT (`symbol`, `venue`) DO UPDATE SET '
        else:
          statement += ' ON DUPLICATE KEY UPDATE '
        statement += ', '.join(updates)
      self.statements[table] = statement

  def store(self, table, rows):
    with self.lock:
      with closing(self.connection.cursor()) as cursor:
        try:
          cursor.execute('BEGIN')
          cursor.executemany(self.statements[table],
            format_rows(self.columns[table], rows))
          self.connection.commit()
        except Exception:
          self.connection.rollback()
          raise

def copy_table(source, destination, table, index, start, end):
  """Copies the inclusive sequence range selected by the timestamp bounds."""
  placeholder = get_placeholder(source)
  if isinstance(index, nexus.Ticker):
    condition = '`symbol` = ' + placeholder + ' AND `venue` = ' + placeholder
    parameters = (index.symbol, str(index.venue.code))
  else:
    condition = '`venue` = ' + placeholder
    parameters = (str(index.code),)
  table_name = quote_identifier(table)
  with closing(source.cursor()) as cursor:
    cursor.execute('SELECT MIN(`query_sequence`) FROM ' + table_name +
      ' WHERE ' + condition + ' AND `timestamp` >= ' + placeholder,
      parameters + (start,))
    first = cursor.fetchone()[0]
    cursor.execute('SELECT MAX(`query_sequence`) FROM ' + table_name +
      ' WHERE ' + condition + ' AND `timestamp` <= ' + placeholder,
      parameters + (end,))
    last = cursor.fetchone()[0]
  if first is None or last is None or first > last:
    return
  columns = destination.columns[table]
  sequence_column = columns.index('query_sequence')
  statement = 'SELECT ' + get_select_columns(source, columns)
  statement += ' FROM ' + table_name + ' WHERE ' + condition + \
    ' AND `query_sequence` >= ' + placeholder + \
    ' AND `query_sequence` <= ' + placeholder + \
    ' ORDER BY `query_sequence` LIMIT ' + str(BATCH_SIZE)
  while first <= last:
    with closing(open_cursor(source)) as cursor:
      cursor.execute(statement, parameters + (first, last))
      rows = cursor.fetchmany(BATCH_SIZE)
    if not rows:
      return
    destination.store(table, rows)
    first = rows[-1][sequence_column] + 1
    del rows

def backup(index, start, end, source_builder, destination):
  start, end = get_range(index, start, end)
  with closing(source_builder()) as source:
    if isinstance(index, nexus.Ticker):
      for table in ('bbo_quotes', 'time_and_sales', 'book_quotes'):
        copy_table(source, destination, table, index, start, end)
    else:
      copy_table(source, destination, 'order_imbalances', index, start, end)

def backup_ticker_info(source, destination):
  columns = destination.columns['ticker_info']
  symbol_column = columns.index('symbol')
  venue_column = columns.index('venue')
  placeholder = get_placeholder(source)
  select = 'SELECT ' + get_select_columns(source, columns)
  select += ' FROM `ticker_info`'
  anchor = None
  while True:
    statement = select
    parameters = ()
    if anchor is not None:
      statement += ' WHERE (`symbol`, `venue`) > (' + \
        placeholder + ', ' + placeholder + ')'
      parameters += anchor
    statement += ' ORDER BY `symbol`, `venue` LIMIT ' + str(BATCH_SIZE)
    with closing(open_cursor(source)) as cursor:
      cursor.execute(statement, parameters)
      rows = cursor.fetchmany(BATCH_SIZE)
    if not rows:
      return
    destination.store('ticker_info', rows)
    anchor = (rows[-1][symbol_column], rows[-1][venue_column])
    del rows

def run_backups(indexes, start, end, source_builder, destination, cores):
  with ThreadPoolExecutor(max_workers=cores) as executor:
    for i in range(0, len(indexes), cores):
      futures = [executor.submit(backup, index, start, end, source_builder,
        destination) for index in indexes[i : i + cores]]
      for future in futures:
        try:
          future.result()
        except Exception:
          traceback.print_exc()

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
  if args.cores < 1:
    parser.error('Number of cores must be positive.')
  try:
    with open(args.config, 'r') as stream:
      config = yaml.load(stream, yaml.SafeLoader)
  except IOError:
    sys.stderr.write('%s not found\n' % args.config)
    sys.exit(1)
  except yaml.YAMLError as e:
    report_yaml_error(e)
    sys.exit(1)
  if args.output and args.input:
    print('Can not use Sqlite as both the input and output.')
    sys.exit(1)
  elif not args.output and not args.input:
    print('Sqlite path not specified.')
    sys.exit(1)
  data_store_config = config['data_store']
  address = parse_ip_address(data_store_config['address'])
  username = data_store_config['username']
  password = data_store_config['password']
  schema = data_store_config['schema']
  mysql_builder = functools.partial(pymysql.connect, host=address.host,
    port=address.port, user=username, password=password, database=schema,
    autocommit=True)
  if args.output is not None:
    sqlite_path = args.output
    source_builder = mysql_builder
  else:
    sqlite_path = args.input
    source_builder = functools.partial(sqlite3.connect, sqlite_path)
  with closing(source_builder()) as source:
    tickers = load_tickers(source)
    venues = load_venues(source)
  with closing(nexus.MySqlHistoricalDataStore(
      address.host, address.port, username, password, schema)):
    pass
  with closing(nexus.SqliteHistoricalDataStore(sqlite_path)):
    pass
  with closing(sqlite3.connect(sqlite_path)) as connection:
    columns = load_columns(connection)
  if args.output is not None:
    destination = sqlite3.connect(sqlite_path, check_same_thread=False)
  else:
    destination = mysql_builder()
  with closing(destination):
    writer = Writer(destination, columns)
    run_backups(tickers, args.start, args.end, source_builder, writer,
      args.cores)
    run_backups(venues, args.start, args.end, source_builder, writer,
      args.cores)
    with closing(source_builder()) as source:
      backup_ticker_info(source, writer)

if __name__ == '__main__':
  main()
