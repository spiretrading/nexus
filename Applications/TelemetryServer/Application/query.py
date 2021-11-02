import argparse
import datetime
import sys

import yaml

import beam
import nexus

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

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file.',
    default='config.yml')
  parser.add_argument('-a', '--account', type=str,
    help='The account to query.', required=True)
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
  section = config['service_locator']
  address = parse_ip_address(section['address'])
  username = section['username']
  password = section['password']
  service_clients = \
    nexus.ApplicationServiceClients(username, password, address)
  telemetry_client = nexus.telemetry_service.ApplicationTelemetryClient(
    service_clients.get_service_locator_client(),
    service_clients.get_time_client())
  query = beam.queries.Query()
  query.index = \
    service_clients.get_service_locator_client().find_account(args.account)
  start = beam.time_service.to_utc_time(args.start)
  end = beam.time_service.to_utc_time(args.end)
  query.range = beam.queries.Range(start, end)
  query.snapshot_limit = beam.queries.SnapshotLimit.UNLIMITED
  queue = beam.Queue()
  telemetry_client.query_telemetry_events(query, queue)
  try:
    while True:
      print(queue.pop())
  except beam.PipeBrokenException:
    pass

if __name__ == '__main__':
  main()
