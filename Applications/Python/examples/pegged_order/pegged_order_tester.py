import beam
import nexus
import pegged_order
import unittest

class TestPeggedOrder(unittest.TestCase):
  def setUp(self):
    self.security = nexus.parse_security('ABX.TSX')
    self.environment = nexus.TestEnvironment()
    self.environment.open()
    self.service_clients = nexus.TestServiceClients(self.environment)
    self.service_clients.open()

  def tearDown(self):
    self.service_clients.close()
    self.environment.close()

  def test_bid(self):
    order_fields = nexus.order_execution_service.OrderFields.build_limit_order(
      self.service_clients.get_service_locator_client().account,
      self.security, nexus.default_currencies.CAD, nexus.Side.BID, 'TSX',
      1000, nexus.Money.ZERO)
    order = pegged_order.PeggedOrder(self.service_clients, order_fields,
      nexus.Money.ZERO)
    order.start()
    bbo = nexus.BboQuote(
      nexus.Quote(nexus.Money.from_value('1.00'), 100, nexus.Side.BID),
      nexus.Quote(nexus.Money.from_value('1.01'), 100, nexus.Side.ASK), None)
    self.environment.update(self.security, bbo)
    submission_queue = beam.Queue()
    self.environment.get_order_execution_instance().get_driver().\
      get_publisher().monitor(submission_queue)
    order = submission_queue.top()
    submission_queue.pop()
    self.assertEqual(order.info.fields.price, nexus.Money.from_value('1.00'))
    print 'yo'
    order.wait()

def main():
  unittest.main()

if __name__ == '__main__':
  main()
