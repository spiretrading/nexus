import beam
import datetime
import nexus
import unittest

class TestOrderExecutionService(unittest.TestCase):
  def setUp(self):

    # Build a test instance of the ServiceLocator.
    self.service_locator_instance = \
      beam.service_locator.tests.ServiceLocatorTestInstance()
    self.service_locator_instance.open()

    # Build a ServiceLocator for testing purposes.
    self.service_locator_client = self.service_locator_instance.build_client()
    self.service_locator_client.set_credentials('root', '')
    self.service_locator_client.open()

    # Build a test instance of the UID service.
    self.uid_instance = beam.uid_service.tests.UidServiceTestInstance()
    self.uid_instance.open()

    # Build a ServiceLocatorClient for use exclusively by the administration
    # service.  Use account 'root'.
    administration_service_locator_client = \
      self.service_locator_instance.build_client()
    administration_service_locator_client.set_credentials('root', '')
    administration_service_locator_client.open()

    # Build a test instance of the AdministrationService using the root
    # ServiceLocatorClient constructed above.
    self.administration_instance = \
      nexus.administration_service.tests.AdministrationServiceTestInstance(
      administration_service_locator_client)
    self.administration_instance.open()

    # Similarly build a ServiceLocatorClient for use exclusively by the market
    # data service using the root account.
    market_data_service_locator_client = \
      self.service_locator_instance.build_client()
    market_data_service_locator_client.set_credentials('root', '')
    market_data_service_locator_client.open()

    # Build a test instance of the MarketDataService using the root account.
    self.market_data_instance = \
      nexus.market_data_service.tests.MarketDataServiceTestInstance(
      market_data_service_locator_client)
    self.market_data_instance.open()

    # Build a ServiceLocatorClient exclusively for the order execution service.
    order_execution_service_locator_client = \
      self.service_locator_instance.build_client()
    order_execution_service_locator_client.set_credentials('root', '')
    order_execution_service_locator_client.open()

    # Build a UID client exclusively for the order execution service.
    order_execution_uid_client = self.uid_instance.build_client()
    order_execution_uid_client.open()

    # Build an administration client exclusively for the order execution
    # service.
    order_execution_administration_client = \
      self.administration_instance.build_client(self.service_locator_client)
    order_execution_administration_client.open()

    # Build a test instance of the OrderExecutionService using the root account.
    self.order_execution_instance = \
      nexus.order_execution_service.tests.OrderExecutionServiceTestInstance(
      order_execution_service_locator_client, order_execution_uid_client,
      order_execution_administration_client)
    self.order_execution_instance.open()

    # Build an order execution client for testing purposes.
    self.order_execution_client = \
      self.order_execution_instance.build_client(self.service_locator_client)
    self.order_execution_client.open()
    query = nexus.order_execution_service.AccountQuery()
    query.index = self.service_locator_client.account
    query.range = beam.queries.Range.REAL_TIME
    self.queue = beam.Queue()
    self.order_execution_client.query_order_submissions(query, self.queue)

  def testFill(self):

    # Listen for incoming order submissions on the server side.
    server_queue = beam.Queue()
    self.order_execution_instance.get_driver().get_publisher().monitor(
      server_queue)

    # On the client side, submit an order on a test symbol.
    security = nexus.parse_security('TST.NYSE')
    order_fields = nexus.order_execution_service.OrderFields.build_limit_order(
      self.service_locator_client.account, security,
      nexus.default_currencies.USD, nexus.Side.BID, 'NYSE', 100,
      nexus.Money.ONE)
    order = self.order_execution_client.submit(order_fields)

    # Extract the order from the server side.
    server_order = server_queue.top()
    server_queue.pop()

    # Listen to the client side order for updates.
    queue = beam.Queue()
    order.get_publisher().monitor(queue)

    # The first execution report should always be PENDING_NEW
    report = queue.top()
    queue.pop()
    self.assertEqual(report.status, nexus.OrderStatus.PENDING_NEW)

    # On the server side, set the order status to NEW
    nexus.order_execution_service.tests.set_order_status(server_order,
      nexus.OrderStatus.NEW, datetime.datetime.utcnow())

    # Verify that the client received the update.
    report = queue.top()
    queue.pop()
    self.assertEqual(report.status, nexus.OrderStatus.NEW)

    # On the server side, fill the order.
    nexus.order_execution_service.tests.fill_order(server_order,
      order_fields.price, order_fields.quantity, datetime.datetime.utcnow())

    # Verify that the client received the fill.
    report = queue.top()
    queue.pop()
    self.assertEqual(report.status, nexus.OrderStatus.FILLED)

def main():
  unittest.main()

if __name__ == "__main__":
  main()
