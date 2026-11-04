import argparse
import calendar
import contextlib
import datetime
import io
from decimal import Decimal
from pathlib import Path
import sys

import beam
import nexus
import yaml

from statement_pdf import write_statement


class Fees:
  AUSTRALIAN_FEE = 24 * nexus.Money.CENT
  CANADIAN_FEE = (185 * nexus.Money.BIP) / 100
  US_RATE = 0.00007

  def __init__(self):
    self.totals = {currency: nexus.Money.ZERO
      for currency in ('CAD', 'AUD', 'USD')}

  def update(self, order):
    if order.info.fields.destination == nexus.destinations.MOE:
      return
    country = nexus.VENUES.select(order.info.fields.ticker.venue).country_code
    if country not in (nexus.countries.CA, nexus.countries.AU,
        nexus.countries.US):
      return
    for execution in order.publisher.get_snapshot():
      if execution.last_quantity == 0 or str(execution.last_market) == \
          str(nexus.destinations.MOE):
        continue
      if country == nexus.countries.CA:
        self.totals['CAD'] += execution.last_quantity * self.CANADIAN_FEE
      elif country == nexus.countries.AU:
        self.totals['AUD'] += self.AUSTRALIAN_FEE
      else:
        notional = execution.last_quantity * execution.last_price
        self.totals['USD'] += nexus.round_to(
          self.US_RATE * notional, nexus.Money.CENT)


def parse_month(value):
  try:
    if len(value) != 8 or not value.isascii() or not value.isdigit():
      raise ValueError()
    month = datetime.datetime.strptime(value, '%Y%m%d').date()
    if month.day != 1 or month.year < 1400 or \
        month == datetime.date(9999, 12, 1):
      raise ValueError()
    return month
  except ValueError as error:
    raise argparse.ArgumentTypeError(
      'Trading month must be a date in YYYYMM01 format.') from error


def load_config(path):
  with path.open(encoding='utf-8') as source:
    config = yaml.safe_load(source)
  if not isinstance(config, dict):
    raise ValueError('Invalid statement service configuration.')
  company = config.get('company')
  if not isinstance(company, dict) or \
      not isinstance(company.get('name'), str) or not company['name'].strip():
    raise ValueError('Company configuration requires a name.')
  company.setdefault('address', [])
  company.setdefault('registrations', [])
  if not isinstance(company['address'], list) or any(
      not isinstance(line, str) for line in company['address']):
    raise ValueError('Company address must be a list of text lines.')
  if not isinstance(company['registrations'], list):
    raise ValueError('Company registrations must be a list.')
  for registration in company['registrations']:
    if not isinstance(registration, dict) or any(
        not isinstance(registration.get(field), str) or
        not registration[field].strip() for field in ('label', 'value')):
      raise ValueError('Each company registration requires a label and value.')
  logo = company.get('logo')
  if logo is not None:
    if not isinstance(logo, str) or not logo.strip():
      raise ValueError('Company logo must be a file path.')
    logo = path.resolve().parent / logo
    if not logo.is_file():
      raise ValueError('Company logo file was not found.')
  company['logo'] = logo
  if isinstance(config.get('service_locator'), dict):
    config = config['service_locator']
    address = config.get('address')
  else:
    address = config.get('service_locator')
  if not isinstance(address, str) or not address or \
      not isinstance(config.get('username'), str) or \
      not isinstance(config.get('password'), str) or \
      not isinstance(config.get('public_key'), str):
    raise ValueError('Statement configuration requires a service locator '
      'address, username, password and public key.')
  host, separator, port = address.rpartition(':')
  if not separator:
    host, port = address, '0'
  return (config['username'], config['password'],
    beam.IpAddress(host, int(port)),
    beam.AesGcmClientConfig(config['public_key']), company)


def load_fees(account, start, end, clients):
  fees = Fees()
  time_zones = clients.definitions_client.load_time_zone_database()
  first = datetime.datetime.combine(start, datetime.time())
  next_month = datetime.datetime.combine(
    end + datetime.timedelta(days=1), datetime.time())
  seen = set()
  for venue in nexus.VENUES.entries:
    if venue.country_code not in (nexus.countries.CA, nexus.countries.AU,
        nexus.countries.US):
      continue
    beginning = nexus.venue_to_utc(venue.venue, first, time_zones)
    ending = nexus.venue_to_utc(venue.venue, next_month, time_zones) - \
      datetime.timedelta(microseconds=1)
    query = nexus.make_daily_order_submission_query(
      venue.venue, account, beginning, ending, time_zones)
    query.range = beam.Range(beginning, ending)
    queue = beam.Queue()
    clients.order_execution_client.query_orders(query, queue)
    while True:
      try:
        order = queue.pop()
      except beam.PipeBrokenException:
        break
      if order.info.fields.account.id != account.id:
        raise ValueError('Order query returned a different account.')
      if order.info.id not in seen:
        seen.add(order.info.id)
        fees.update(order)
  return fees


def make_statement(account, identity, month, fees):
  end = month.replace(day=calendar.monthrange(month.year, month.month)[1])
  billing_date = (end + datetime.timedelta(days=1)).replace(day=15)
  country = nexus.COUNTRIES.from_code(identity.country).name
  lines = [value.strip() for value in (identity.address_line_one,
    identity.address_line_two, identity.address_line_three) if value.strip()]
  locality = ', '.join(value.strip() for value in
    (identity.city, identity.province) if value.strip())
  if locality:
    lines.append(locality)
  if country:
    lines.append(country)
  details = {
    'account_name': account.name,
    'account_holder_name': ' '.join(value.strip() for value in
      (identity.first_name, identity.last_name) if value.strip()),
    'account_email': identity.email_address.strip(),
    'address': '\n'.join(lines),
    'trading_period_start': month.isoformat(),
    'trading_period_end': end.isoformat()
  }
  totals = {currency: Decimal(str(value))
    for currency, value in fees.totals.items()}
  markets = {'CAD': 'Canadian', 'AUD': 'Australian', 'USD': 'US'}
  items = [('Spire licence fees - ' + markets[currency] + ' markets',
    currency, total) for currency, total in totals.items()]
  return (account.id, billing_date), {
    'details': details,
    'items': items,
    'totals': totals
  }


def generate(account_id, month, config_path):
  username, password, address, encryption, company = load_config(config_path)
  clients = nexus.ServiceClients(username, password, address, encryption)
  try:
    account = clients.service_locator_client.load_directory_entry(account_id)
    if account.type != beam.DirectoryEntry.Type.ACCOUNT:
      raise ValueError('The statement owner must be an account.')
    nexus.load_definitions(clients.definitions_client)
    end = month.replace(day=calendar.monthrange(month.year, month.month)[1])
    fees = load_fees(account, month, end, clients)
    identity = clients.administration_client.load_identity(account)
    key, statement = make_statement(account, identity, month, fees)
    output = io.BytesIO()
    write_statement(output, key, statement, company)
    return output.getvalue()
  finally:
    clients.close()


def main():
  parser = argparse.ArgumentParser(
    description='Generate one account statement for one trading month.')
  parser.add_argument('--account-id', type=int, required=True,
    help='Trusted account ID supplied by the report definition.')
  parser.add_argument('--month', type=parse_month, required=True,
    help='Trading month as YYYYMM01.')
  parser.add_argument('--config', type=Path,
    default=Path(__file__).with_name('config.yml'),
    help='Server-side service configuration.')
  args = parser.parse_args()
  if args.account_id < 0:
    parser.error('Account ID must not be negative.')
  try:
    with contextlib.redirect_stdout(sys.stderr):
      output = generate(args.account_id, args.month, args.config)
    sys.stdout.buffer.write(output)
    sys.stdout.buffer.flush()
  except Exception as error:
    print('Statement generation failed: ' + str(error), file=sys.stderr)
    return 1
  return 0


if __name__ == '__main__':
  sys.exit(main())
