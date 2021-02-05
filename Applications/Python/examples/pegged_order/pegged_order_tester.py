import aspen
import beam
import nexus
import pegged_order
import unittest

class TestPeggedOrder(unittest.TestCase):
  def setUp(self):
    self.environment = nexus.TestEnvironment()
    self.service_clients = nexus.TestServiceClients(self.environment)

  # Start a PeggedOrder on the bid for 1000 shares with an offset of 0.01.
  # Set the BBO to 1.00 / 1.01.
  # Expect a limit bid order submission for 0.99 for 1000 shares.
  # Reject the order submission.
  # Expect the PeggedOrder to terminate.
  def test_rejection(self):
    security = nexus.parse_security('ABX.TSX')
    order_fields = nexus.order_execution_service.OrderFields.make_limit_order(
      security, nexus.Side.BID, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(self.service_clients, order_fields,
      nexus.Money.CENT)
    order.start()
    self.environment.update_bbo_price(security,
      nexus.Money.from_value('1.00'), nexus.Money.from_value('1.01'))
    submission_queue = beam.Queue()
    self.environment.monitor_order_submissions(submission_queue)
    expected_order = submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('0.99'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.reject(expected_order)
    order.wait()
    self.assertIsNone(submission_queue.try_pop())

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
    order_fields = nexus.order_execution_service.OrderFields.make_limit_order(
      security, nexus.Side.ASK, 1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(self.service_clients, order_fields,
      nexus.Money.CENT)
    order.start()
    self.environment.update_bbo_price(security, nexus.Money.from_value('1.00'),
       nexus.Money.from_value('1.01'))
    submission_queue = beam.Queue()
    self.environment.monitor_order_submissions(submission_queue)
    expected_order = submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('1.02'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    self.environment.update_bbo_price(security,
      nexus.Money.from_value('0.90'), nexus.Money.from_value('0.91'))
    self.assertTrue(nexus.order_execution_service.tests.is_pending_cancel(
      expected_order))
    self.environment.cancel(expected_order)
    expected_order = submission_queue.pop()
    self.assertEqual(expected_order.info.fields.price,
      nexus.Money.from_value('0.92'))
    self.assertEqual(expected_order.info.fields.quantity, 1000)
    self.environment.accept(expected_order)
    self.environment.fill(expected_order, 1000)
    order.wait()
    self.assertIsNone(submission_queue.try_pop())

def main():
  unittest.main()

if __name__ == '__main__':
  main()
