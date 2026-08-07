import unittest

import beam
import nexus

import pegged_order

class TestPeggedOrder(unittest.TestCase):
  def setUp(self):
    self.environment = nexus.TestEnvironment()
    self.clients = nexus.TestClients(self.environment)
    self.ticker = nexus.parse_ticker('ABX.TSX')
    self.submissions = beam.Queue()
    self.environment.monitor_order_submissions(self.submissions)

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit bid order submission for 0.99 for 1000 shares, published
  # on the orders publisher.
  # Accept and fill the order submission.
  # Expect the PeggedOrder to terminate and close its orders publisher.
  def test_fill(self):
    order_fields = nexus.make_market_order_fields(
      self.ticker, nexus.Side.BID, 1000)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    orders = beam.Queue()
    order.orders.monitor(orders)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(expected_order.info.fields.type, nexus.OrderType.LIMIT)
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('0.99'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.assertEqual(expected_order.info.fields.side, nexus.Side.BID)
    published_order = orders.pop()
    self.assertEqual(
      published_order.info.id, expected_order.info.id)
    self.environment.accept(expected_order)
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertRaises(beam.PipeBrokenException, orders.pop)
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Reject the order submission.
  # Expect the PeggedOrder to terminate and close its orders publisher.
  def test_rejection(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    orders = beam.Queue()
    order.orders.monitor(orders)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.environment.reject(expected_order)
    order.wait()
    orders.pop()
    self.assertRaises(RuntimeError, orders.pop)
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the ask for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit ask order submission for 1.02 for 1000 shares.
  # Accept the order submission.
  # Decrease the BBO to 0.90 / 0.91.
  # Expect a cancel request.
  # Cancel the order submission.
  # Expect a new order submission for 0.92 for 1000 shares.
  # Accept and fill the order submission.
  # Expect the PeggedOrder to terminate.
  def test_reprice(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.ASK, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('1.02'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    reports = beam.Queue()
    expected_order.publisher.monitor(reports)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('0.90'), nexus.Money.parse('0.91'))
    while reports.pop().status != nexus.OrderStatus.PENDING_CANCEL:
      pass
    self.environment.cancel(expected_order)
    expected_order = self.submissions.pop()
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('0.92'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares with a limit price of
  # 0.95 and a peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit bid order submission capped at 0.95.
  # Accept the order submission.
  # Increase the BBO to 1.05 / 1.06.
  # Expect no resubmission since the peg saturates at the limit price.
  # Fill the order submission.
  # Expect the PeggedOrder to terminate.
  def test_limit_price(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.parse('0.95'))
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('0.95'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.05'), nexus.Money.parse('1.06'))
    self.assertIsNone(self.submissions.try_pop())
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Accept the order submission for 0.99.
  # Decrease the BBO to 0.95 / 0.96.
  # Expect no cancel request and no resubmission since the market moved
  # toward the order.
  # Fill the order submission.
  # Expect the PeggedOrder to terminate.
  def test_toward_move_holds(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('0.99'))
    self.environment.accept(expected_order)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('0.95'), nexus.Money.parse('0.96'))
    self.assertIsNone(self.submissions.try_pop())
    self.assertFalse(nexus.tests.is_pending_cancel(expected_order))
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares.
  # Set the BBO to 1.00 / 1.01.
  # Cancel the PeggedOrder while the order submission is pending.
  # Accept the order submission.
  # Expect a cancel request once the order submission is accepted.
  # Cancel the order submission.
  # Expect the PeggedOrder to terminate.
  def test_cancel_pending_new(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    reports = beam.Queue()
    expected_order.publisher.monitor(reports)
    order.cancel()
    self.environment.accept(expected_order)
    while reports.pop().status != nexus.OrderStatus.PENDING_CANCEL:
      pass
    self.environment.cancel(expected_order)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares.
  # Set the BBO to 1.00 / 1.01.
  # Cancel the PeggedOrder while the order submission is pending.
  # Reject the order submission.
  # Expect the PeggedOrder to terminate with nothing to cancel.
  def test_cancel_pending_new_rejected(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    order.cancel()
    self.environment.reject(expected_order)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Accept and partially fill the order submission for 400 shares.
  # Increase the BBO to 1.05 / 1.06.
  # Expect a cancel request.
  # Cancel the order submission.
  # Expect a new order submission for 1.04 for the remaining 600 shares.
  # Accept and fill the order submission.
  # Expect the PeggedOrder to terminate.
  def test_partial_fills(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    self.environment.fill(expected_order, 400)
    reports = beam.Queue()
    expected_order.publisher.monitor(reports)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.05'), nexus.Money.parse('1.06'))
    while reports.pop().status != nexus.OrderStatus.PENDING_CANCEL:
      pass
    self.environment.cancel(expected_order)
    expected_order = self.submissions.pop()
    self.assertEqual(
      expected_order.info.fields.price, nexus.Money.parse('1.04'))
    self.assertEqual(expected_order.info.fields.quantity, 600)
    self.environment.accept(expected_order)
    self.environment.fill(expected_order, 600)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Accept the order submission.
  # Cancel the PeggedOrder.
  # Expect a cancel request.
  # Cancel the order submission.
  # Expect the PeggedOrder to terminate with no further submissions.
  def test_cancel(self):
    order_fields = nexus.make_limit_order_fields(
      self.ticker, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.environment.accept(expected_order)
    reports = beam.Queue()
    expected_order.publisher.monitor(reports)
    order.cancel()
    while reports.pop().status != nexus.OrderStatus.PENDING_CANCEL:
      pass
    self.environment.cancel(expected_order)
    order.wait()
    self.assertIsNone(self.submissions.try_pop())

def main():
  unittest.main()

  # Start a PeggedOrder on the bid for 1000 shares with no limit price and a
  # peg difference of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit bid order submission for 0.99 for 1000 shares.
  # Fill the order submission without accepting it first.
  # Expect the PeggedOrder to terminate and close its orders publisher.
  def test_fill_without_new(self):
    order_fields = nexus.make_market_order_fields(
      self.ticker, nexus.Side.BID, 1000)
    order = pegged_order.PeggedOrder(
      self.clients, order_fields, nexus.Money.CENT)
    orders = beam.Queue()
    order.orders.monitor(orders)
    self.environment.update_bbo_price(
      self.ticker, nexus.Money.parse('1.00'), nexus.Money.parse('1.01'))
    expected_order = self.submissions.pop()
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertEqual(orders.pop().info.id, expected_order.info.id)
    self.assertRaises(beam.PipeBrokenException, orders.pop)
    self.assertIsNone(self.submissions.try_pop())

if __name__ == '__main__':
  main()
