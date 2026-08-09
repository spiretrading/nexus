import argparse
import json
import struct
import sys

import nexus
import pymysql
import pymysql.cursors
import yaml

def to_ordinals(enumeration, predicate=lambda value: True):
  return tuple(sorted(int(value)
    for value in enumeration.__members__.values() if predicate(value)))

def enum_domain(enumeration):
  ordinals = to_ordinals(enumeration)
  return ordinals[0], ordinals[-1]

DBL_MIN = sys.float_info.min
DBL_MAX = sys.float_info.max
NONE_SIDE = int(nexus.Side.NONE)
PENDING_NEW = int(nexus.OrderStatus.PENDING_NEW)
FILLED = int(nexus.OrderStatus.FILLED)
TERMINAL_STATUSES = to_ordinals(nexus.OrderStatus,
  lambda value: value != nexus.OrderStatus.NONE and nexus.is_terminal(value))
FILL_STATUSES = (
  int(nexus.OrderStatus.PARTIALLY_FILLED), int(nexus.OrderStatus.FILLED))
NUMERIC_COLUMNS = (
  ('submissions', 'quantity'),
  ('submissions', 'price'),
  ('execution_reports', 'last_quantity'),
  ('execution_reports', 'last_price'),
  ('execution_reports', 'execution_fee'),
  ('execution_reports', 'processing_fee'),
  ('execution_reports', 'commission'))
ENUM_COLUMNS = (
  ('submissions', 'side', nexus.Side),
  ('submissions', 'type', nexus.OrderType),
  ('submissions', 'time_in_force', nexus.TimeInForce.Type),
  ('execution_reports', 'status', nexus.OrderStatus))

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' %
      (error.problem_mark.line, error.problem_mark.column, error.problem))
  else:
    sys.stderr.write('Invalid YAML provided\n')

def parse_address(source):
  separator = source.find(':')
  if separator == -1:
    return source, 3306
  return source[:separator], int(source[separator + 1:])

def to_bits(value):
  return struct.unpack('<q', struct.pack('<d', value))[0]

def to_list(values):
  return ', '.join(str(value) for value in values)

def scope(table, account):
  if account is None:
    return ''
  return f' AND {table}.account = {account}'

def row_key(table):
  if table == 'execution_reports':
    return ('order_id', 'sequence')
  return ('order_id',)

def make_check(identifier, name, table, key, query):
  return {'id': identifier, 'name': name, 'table': table, 'key': key,
    'query': query}

def make_checks(account):
  terminal = to_list(TERMINAL_STATUSES)
  fills = to_list(FILL_STATUSES)
  checks = [
    make_check('missing_execution_reports',
      'submissions without execution reports', 'submissions', ('order_id',), f'''
      SELECT s.order_id FROM submissions s
        WHERE NOT EXISTS (
          SELECT 1 FROM execution_reports r WHERE r.order_id = s.order_id)
          {scope('s', account)}'''),
    make_check('report_after_terminal',
      'execution reports following a terminal status', 'execution_reports',
      ('order_id', 'sequence'), f'''
      SELECT r.order_id, r.sequence, r.status FROM execution_reports r
        JOIN (
          SELECT order_id, MIN(sequence) AS terminal_sequence
            FROM execution_reports WHERE status IN ({terminal})
            GROUP BY order_id) t ON t.order_id = r.order_id
        WHERE r.sequence > t.terminal_sequence{scope('r', account)}'''),
    make_check('live_order_not_tracked',
      'non terminal submissions absent from live_orders', 'submissions',
      ('order_id',), f'''
      SELECT s.order_id, r.status FROM submissions s
        JOIN (
          SELECT order_id, MAX(sequence) AS last_sequence
            FROM execution_reports GROUP BY order_id) m
          ON m.order_id = s.order_id
        JOIN execution_reports r
          ON r.order_id = s.order_id AND r.sequence = m.last_sequence
        WHERE r.status NOT IN ({terminal})
          AND NOT EXISTS (
            SELECT 1 FROM live_orders l WHERE l.order_id = s.order_id)
          {scope('s', account)}'''),
    make_check('first_report_not_pending_new',
      'first execution report is not PENDING_NEW', 'execution_reports',
      ('order_id', 'sequence'), f'''
      SELECT r.order_id, r.sequence, r.status FROM execution_reports r
        JOIN (
          SELECT order_id, MIN(sequence) AS first_sequence
            FROM execution_reports GROUP BY order_id) f
          ON f.order_id = r.order_id AND f.first_sequence = r.sequence
        WHERE r.status <> {PENDING_NEW}{scope('r', account)}'''),
    make_check('overfilled_order',
      'executed quantity exceeds the submitted quantity', 'submissions',
      ('order_id',), f'''
      SELECT s.order_id, s.quantity, t.filled FROM submissions s
        JOIN (
          SELECT order_id, SUM(last_quantity) AS filled
            FROM execution_reports GROUP BY order_id) t
          ON t.order_id = s.order_id
        WHERE t.filled > s.quantity{scope('s', account)}'''),
    make_check('filled_order_short',
      'FILLED orders whose executed quantity is short', 'submissions',
      ('order_id',), f'''
      SELECT s.order_id, s.quantity, t.filled FROM submissions s
        JOIN (
          SELECT order_id, MAX(sequence) AS last_sequence,
              SUM(last_quantity) AS filled
            FROM execution_reports GROUP BY order_id) t
          ON t.order_id = s.order_id
        JOIN execution_reports r
          ON r.order_id = s.order_id AND r.sequence = t.last_sequence
        WHERE r.status = {FILLED} AND
          t.filled <> s.quantity{scope('s', account)}'''),
    make_check('report_without_submission',
      'execution reports without a submission', 'execution_reports',
      ('order_id',), f'''
      SELECT DISTINCT r.order_id FROM execution_reports r
        WHERE NOT EXISTS (
          SELECT 1 FROM submissions s WHERE s.order_id = r.order_id)
          {scope('r', account)}'''),
    make_check('live_order_without_submission',
      'live_orders entries without a submission', 'live_orders', ('order_id',),
      '''
      SELECT l.order_id FROM live_orders l
        WHERE NOT EXISTS (
          SELECT 1 FROM submissions s WHERE s.order_id = l.order_id)'''),
    make_check('report_sequence_broken',
      'execution report sequences with a gap or a duplicate',
      'execution_reports', ('order_id',), f'''
      SELECT order_id, COUNT(*) AS reports,
          COUNT(DISTINCT sequence) AS distinct_sequences,
          MIN(sequence) AS first_sequence, MAX(sequence) AS last_sequence
        FROM execution_reports
        WHERE 1 = 1{scope('execution_reports', account)}
        GROUP BY order_id
        HAVING first_sequence <> 0 OR last_sequence <> reports - 1
          OR distinct_sequences <> reports'''),
    make_check('execution_on_non_fill_status',
      'executed quantity on a status that carries no execution',
      'execution_reports', ('order_id', 'sequence'), f'''
      SELECT order_id, sequence, status, last_quantity FROM execution_reports
        WHERE (last_quantity < 0 OR
          (last_quantity > 0 AND status NOT IN ({fills})))
          {scope('execution_reports', account)}'''),
    make_check('submission_without_side', 'submissions without a side',
      'submissions', ('order_id',), f'''
      SELECT order_id, side FROM submissions
        WHERE side = {NONE_SIDE}{scope('submissions', account)}'''),
    make_check('submission_without_quantity',
      'submissions without a positive quantity', 'submissions', ('order_id',),
      f'''
      SELECT order_id, quantity FROM submissions
        WHERE quantity <= 0{scope('submissions', account)}''')]
  for table, column in NUMERIC_COLUMNS:
    key = row_key(table)
    columns = ', '.join(f'`{name}`' for name in key)
    checks.append(make_check(f'unreadable_value_{table}_{column}',
      f'unreadable values in {table}.{column}', table, key, f'''
      SELECT {columns}, `{column}` FROM {table}
        WHERE (`{column}` IS NULL OR (`{column}` <> 0 AND
          (ABS(`{column}`) < {DBL_MIN} OR ABS(`{column}`) > {DBL_MAX})))
          {scope(table, account)}'''))
  for table, column, enumeration in ENUM_COLUMNS:
    key = row_key(table)
    columns = ', '.join(f'`{name}`' for name in key)
    minimum, maximum = enum_domain(enumeration)
    checks.append(make_check(f'enum_out_of_domain_{table}_{column}',
      f'{table}.{column} outside its domain', table, key, f'''
      SELECT {columns}, `{column}` FROM {table}
        WHERE (`{column}` < {minimum} OR `{column}` > {maximum})
          {scope(table, account)}'''))
  for table in ('submissions', 'execution_reports'):
    checks.append(make_check(f'duplicate_query_sequence_{table}',
      f'duplicate query_sequence in {table}', table,
      ('account', 'query_sequence'), f'''
      SELECT account, query_sequence, COUNT(*) AS occurrences FROM {table}
        WHERE 1 = 1{scope(table, account)}
        GROUP BY account, query_sequence HAVING occurrences > 1'''))
  return checks

def make_findings(check, rows):
  findings = []
  for row in rows:
    key = {name: row[name] for name in check['key'] if name in row}
    values = {name: value for name, value in row.items() if name not in key}
    finding = {'check': check['id'], 'table': check['table'], 'key': key,
      'values': values}
    if check['id'].startswith('unreadable_value_'):
      for name, value in values.items():
        if isinstance(value, float):
          finding['bits'] = to_bits(value)
    findings.append(finding)
  return findings

def format_row(row):
  return ', '.join(f'{name}={value}' for name, value in row.items())

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  parser.add_argument('-a', '--account', type=int,
    help='Account id to restrict the checks to.')
  parser.add_argument('-l', '--limit', type=int, default=20,
    help='Maximum number of rows to report per check, 0 for no maximum.')
  parser.add_argument('-j', '--json', type=str,
    help='File to write the findings to as JSON, - for standard output.')
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
  section = config['data_store']
  host, port = parse_address(section['address'])
  connection = pymysql.connect(host=host, port=port,
    user=section['username'], password=section['password'],
    database=section['schema'], cursorclass=pymysql.cursors.DictCursor)
  findings = []
  try:
    with connection.cursor() as cursor:
      for check in make_checks(args.account):
        cursor.execute(check['query'])
        rows = cursor.fetchall()
        if not rows:
          continue
        findings.extend(make_findings(check, rows))
        if args.json is None:
          print(f'{check["name"]}: {len(rows)}')
          shown = rows if args.limit == 0 else rows[:args.limit]
          for row in shown:
            print('  ' + format_row(row))
          if len(rows) > len(shown):
            print(f'  ... {len(rows) - len(shown)} more')
  finally:
    connection.close()
  if args.json is not None:
    if args.json == '-':
      destination = sys.stdout
    else:
      destination = open(args.json, 'w')
    try:
      for finding in findings:
        destination.write(json.dumps(finding, default=str) + '\n')
    finally:
      if destination is not sys.stdout:
        destination.close()
  if findings:
    exit(1)

if __name__ == '__main__':
  main()
