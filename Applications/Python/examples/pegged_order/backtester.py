"""Backtest the pegged order example.

Usage:
  python backtester.py --db 2026-05-06.db --date 2026-05-06 \
      --ticker ABX.TSX --side bid --start 10:30 --quantity 1000 \
      (--end 15:30 | --duration 18000) [--peg-difference 0.00]

Runs a PeggedOrder over the given window, cancelling whatever remains
unfilled at the end, and reports the ticker, the quantity filled and the
average fill price.
"""
import argparse
import datetime

import beam
import nexus

import pegged_order


def local_datetime(date, hour_minute):
  hour, minute = (int(part) for part in hour_minute.split(':'))
  return datetime.datetime(date.year, date.month, date.day, hour, minute)


def run_backtest(arguments):
  ticker = nexus.parse_ticker(arguments.ticker)
  venue = ticker.venue
  start = nexus.venue_to_utc(
    venue, local_datetime(arguments.date, arguments.start))
  if arguments.end is not None:
    end = nexus.venue_to_utc(
      venue, local_datetime(arguments.date, arguments.end))
  else:
    end = start + datetime.timedelta(seconds=arguments.duration)
  data_store = nexus.SqliteHistoricalDataStore(arguments.db)
  test_environment = nexus.TestEnvironment(data_store)
  test_clients = nexus.TestClients(test_environment)
  environment = nexus.BacktesterEnvironment(start, end, test_clients)
  clients = nexus.BacktesterClients(environment)
  side = nexus.Side.BID if arguments.side == 'bid' else nexus.Side.ASK
  order_fields = nexus.make_market_order_fields(
    ticker, side, arguments.quantity)
  order = pegged_order.PeggedOrder(
    clients, order_fields, nexus.Money.parse(arguments.peg_difference))
  environment.wait()
  order.cancel()
  order.wait()
  portfolio = nexus.make_portfolio(clients)
  print('ticker,filled_quantity,average_price')
  entries = portfolio.entries.items()
  if not entries:
    print('%s,0,%s' % (arguments.ticker, nexus.Money.ZERO))
    return
  for entry_ticker, entry in entries:
    inventory = portfolio.bookkeeper.get_inventory(entry_ticker)
    quantity = inventory.position.quantity
    if quantity == 0:
      average_price = nexus.Money.ZERO
    else:
      average_price = inventory.position.cost_basis / quantity
    print('%s,%s,%s' % (entry_ticker, inventory.volume, average_price))


def parse_arguments():
  parser = argparse.ArgumentParser(
    description=__doc__,
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument(
    '--db', required=True, help='daily SQLite market-data file')
  parser.add_argument(
    '--date', required=True, type=datetime.date.fromisoformat,
    help='trading date, YYYY-MM-DD')
  parser.add_argument(
    '--ticker', required=True, help='ticker to trade, e.g. ABX.TSX')
  parser.add_argument(
    '--side', required=True, choices=['bid', 'ask'], help='side to trade')
  parser.add_argument(
    '--start', required=True, help='window start, local exchange HH:MM')
  window = parser.add_mutually_exclusive_group(required=True)
  window.add_argument('--end', help='window end, local exchange HH:MM')
  window.add_argument(
    '--duration', type=float, help='duration of the order, seconds')
  parser.add_argument(
    '--quantity', required=True, type=int, help='quantity to execute')
  parser.add_argument(
    '--peg-difference', default='0.00', help='price offset from the bbo price')
  return parser.parse_args()


def main():
  run_backtest(parse_arguments())


if __name__ == '__main__':
  main()
