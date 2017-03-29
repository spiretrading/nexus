import beam
import nexus
import pegged_order
import unittest

class TestPeggedOrder(unittest.TestCase):
  def setUp(self):
    self.environment = nexus.TestEnvironment()
    self.environment.open()
    self.service_clients = nexus.TestServiceClients(self.environment)
    self.service_clients.open()

  def tearDown(self):
    self.service_clients.close()
    self.environment.close()

  # Start a PeggedOrder on the bid for 1000 shares with an offset of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit bid order submission for 0.99 for 1000 shares.
  # Reject the order submission.
  # Expect the PeggedOrder to terminate.
  def test_rejection(self):
    security = nexus.parse_security('ABX.TSX')
    order_fields = nexus.order_execution_service.OrderFields.build_limit_order(
      self.service_clients.get_service_locator_client().account, security,
      nexus.default_currencies.CAD, nexus.Side.BID, 'TSX', 1000,
      nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(self.service_clients, order_fields,
      nexus.Money.CENT)
    order.start()
    bbo = nexus.BboQuote(
      nexus.Quote(nexus.Money.from_value('1.00'), 100, nexus.Side.BID),
      nexus.Quote(nexus.Money.from_value('1.01'), 100, nexus.Side.ASK), None)
    self.environment.update(security, bbo)
    submission_queue = beam.Queue()
    self.environment.monitor_order_submissions(submission_queue)
    expected_order = submission_queue.top()
    submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('0.99'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.reject_order(expected_order)
    order.wait()
    self.assertTrue(submission_queue.is_empty())

  # Start a PeggedOrder on the ask for 1000 shares with an offset of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit ask order submission for 1.02 for 1000 shares.
  # Accept the order submission.
  # Decrease the BBO to 0.90 / 0.91.
  # Expect a cancel request.
  # Cancel the order submission.
  # Expect a new order submission for 0.92 for 1000 shares.
  # Accept the order submission.
  # Fill the order submission for 1000 shares.
  # Expect the PeggedOrder to terminate.
  def test_price_retreat(self):
    security = nexus.parse_security('ABX.TSX')
    order_fields = nexus.order_execution_service.OrderFields.build_limit_order(
      self.service_clients.get_service_locator_client().account, security,
      nexus.default_currencies.CAD, nexus.Side.ASK, 'TSX', 1000,
      nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(self.service_clients, order_fields,
      nexus.Money.CENT)
    order.start()
    bbo = nexus.BboQuote(
      nexus.Quote(nexus.Money.from_value('1.00'), 100, nexus.Side.BID),
      nexus.Quote(nexus.Money.from_value('1.01'), 100, nexus.Side.ASK), None)
    self.environment.update(security, bbo)
    submission_queue = beam.Queue()
    self.environment.monitor_order_submissions(submission_queue)
    expected_order = submission_queue.top()
    submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('1.02'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept_order(expected_order)
    bbo = nexus.BboQuote(
      nexus.Quote(nexus.Money.from_value('0.90'), 100, nexus.Side.BID),
      nexus.Quote(nexus.Money.from_value('0.91'), 100, nexus.Side.ASK), None)
    self.environment.update(security, bbo)
    self.assertTrue(nexus.order_execution_service.tests.is_pending_cancel(
      expected_order))
    self.environment.cancel_order(expected_order)
    expected_order = submission_queue.top()
    submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('0.92'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept_order(expected_order)
    self.environment.fill_order(expected_order, 1000)
    order.wait()
    self.assertTrue(submission_queue.is_empty())

def main():
  unittest.main()

if __name__ == '__main__':
  main()
