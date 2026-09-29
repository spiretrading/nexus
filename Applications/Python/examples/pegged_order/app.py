"""Run a pegged order against a live Nexus installation.

Copy config.default.yml to config.yml and fill in the service locator address
and trading account credentials before running:

  python app.py --ticker ABX.TSX --side bid --quantity 1000 \
    --price 25.00 --peg-difference 0.01

The order runs until filled, rejected, or interrupted. On interruption, any
outstanding order is canceled before the service clients are closed.
"""
import argparse
import sys
import time

import beam
import nexus
import yaml

import pegged_order


def parse_ip_address(source):
  separator = source.find(':')
  if separator == -1:
    return beam.IpAddress(source, 0)
  return beam.IpAddress(source[:separator], int(source[separator + 1:]))


def parse_quantity(source):
  quantity = nexus.Quantity.parse(source)
  if quantity <= 0:
    raise argparse.ArgumentTypeError('Quantity must be positive.')
  return quantity


def parse_price(source):
  price = nexus.Money.parse(source)
  if price < nexus.Money.ZERO:
    raise argparse.ArgumentTypeError('Price must not be negative.')
  return price


class Termination:
  """Monitors order completion without blocking shutdown signals."""

  def __init__(self, order):
    self.closed = False
    self.failure = None
    self._tasks = beam.RoutineTaskQueue()
    order.orders.monitor(self._tasks.get_slot(self._on_order, self._on_close))

  def close(self):
    self._tasks.close()
    self._tasks.wait()

  def _on_order(self, order):
    pass

  def _on_close(self, exception):
    if not isinstance(exception, beam.PipeBrokenException):
      self.failure = exception
    self.closed = True


def run_order(clients, order_fields, peg_difference):
  order = pegged_order.PeggedOrder(clients, order_fields, peg_difference)
  termination = Termination(order)
  try:
    while not termination.closed and not beam.received_kill_event():
      time.sleep(0.1)
  except KeyboardInterrupt:
    pass
  finally:
    try:
      order.cancel()
      order.wait()
    finally:
      termination.close()
  if termination.failure is not None:
    raise termination.failure
  return order.filled_quantity


def main():
  parser = argparse.ArgumentParser(
    description=__doc__,
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-c', '--config', default='config.yml',
    help='Configuration file')
  parser.add_argument('-t', '--ticker', required=True,
    help='Ticker to trade, e.g. ABX.TSX')
  parser.add_argument('--side', required=True, choices=['bid', 'ask'],
    help='Side to trade')
  parser.add_argument('-q', '--quantity', required=True, type=parse_quantity,
    help='Quantity to execute')
  parser.add_argument('-p', '--price', type=parse_price,
    default=nexus.Money.ZERO, help='Limit price; zero means no limit')
  parser.add_argument('--peg-difference', type=nexus.Money.parse,
    default=nexus.Money.ZERO,
    help='Price offset below the bid or above the ask')
  parser.add_argument('-d', '--destination',
    help='Order destination; defaults to the ticker venue routing')
  arguments = parser.parse_args()
  try:
    with open(arguments.config, encoding='utf-8') as file:
      config = yaml.safe_load(file)
  except OSError as error:
    parser.error(f'Could not read {arguments.config}: {error.strerror}')
  except yaml.YAMLError:
    parser.error(f'Invalid YAML in {arguments.config}')
  try:
    section = config['service_locator']
    address = parse_ip_address(section['address'])
    username = section['username']
    password = section['password']
    if not isinstance(username, str) or not isinstance(password, str):
      raise TypeError()
  except (KeyError, TypeError, ValueError, AttributeError, OverflowError):
    parser.error('service_locator must specify an address, username, and '
      'password as strings')
  try:
    clients = nexus.ServiceClients(username, password, address)
  except beam.ConnectException as error:
    sys.stderr.write(f'Could not connect to {address} as {username}: '
      f'{error}\n')
    return 1
  try:
    nexus.load_definitions(clients.definitions_client)
    ticker = nexus.parse_ticker(arguments.ticker)
    if not ticker:
      parser.error(f'Invalid ticker {arguments.ticker}')
    side = nexus.Side.BID if arguments.side == 'bid' else nexus.Side.ASK
    fields = nexus.make_limit_order_fields(
      ticker, side, arguments.quantity, arguments.price)
    if arguments.destination is not None:
      fields.destination = arguments.destination
    filled_quantity = run_order(clients, fields, arguments.peg_difference)
    print(f'{ticker}: filled {filled_quantity} of {arguments.quantity}')
  except Exception as error:
    sys.stderr.write(f'Pegged order failed: {error}\n')
    return 1
  finally:
    clients.close()
  return 0


if __name__ == '__main__':
  sys.exit(main())
