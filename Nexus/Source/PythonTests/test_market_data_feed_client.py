from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
import gc
import unittest
import beam
import nexus


class TestMarketDataFeedClient(unittest.TestCase):
    def setUp(self):
        self.operations = beam.Queue()
        self.client = nexus.tests.TestMarketDataFeedClient(self.operations)
        self.executor = ThreadPoolExecutor(max_workers=2)
        self.addCleanup(self.executor.shutdown)
        self.addCleanup(self.operations.close)
        self.addCleanup(self.client.close)

    def pop_operation(self, operation_type):
        operation = self.executor.submit(self.operations.pop).result(timeout=5)
        self.assertIsInstance(operation, operation_type)
        return operation

    def test_add_order(self):
        ticker = nexus.parse_ticker('BHP.ASX')
        venue = nexus.Venue('CXA')
        price = nexus.Money.parse('42.75')
        size = nexus.Quantity(600)
        timestamp = datetime(2026, 9, 14, 10, 0, 0, 123456)
        result = self.executor.submit(self.client.add_order, ticker, venue,
            '1234', True, 'order-123', nexus.Side.ASK, price, size, timestamp)
        operation = self.pop_operation(
            nexus.tests.TestMarketDataFeedClient.AddOrderOperation)
        self.assertEqual(operation.ticker, ticker)
        self.assertEqual(operation.venue, venue)
        self.assertEqual(operation.mpid, '1234')
        self.assertTrue(operation.is_primary_mpid)
        self.assertEqual(operation.id, 'order-123')
        self.assertEqual(operation.side, nexus.Side.ASK)
        self.assertEqual(operation.price, price)
        self.assertEqual(operation.size, size)
        self.assertEqual(operation.timestamp, timestamp)
        self.assertFalse(result.done())
        operation.result.set()
        self.assertIsNone(result.result(timeout=5))

    def test_publish(self):
        client = nexus.tests.TestMarketDataFeedClient
        ticker = nexus.parse_ticker('BHP.ASX')
        venue = nexus.Venue('CXA')
        price = nexus.Money.parse('42.75')
        size = nexus.Quantity(600)
        timestamp = datetime(2026, 9, 14, 10, 0, 0, 123456)
        bid = nexus.Quote(price, size, nexus.Side.BID)
        ask = nexus.Quote(nexus.Money.parse('42.76'), size, nexus.Side.ASK)
        status = nexus.TickerStatus()
        status.venue = venue
        status.state = 'T'
        status.flags = nexus.TickerStatus.Flag.IS_CONTINUOUS
        status.timestamp = timestamp
        cases = [
            (client.PublishOrderImbalanceOperation, 'imbalance',
                beam.IndexedValue(nexus.OrderImbalance(ticker, nexus.Side.BID,
                    size, price, timestamp), venue)),
            (client.PublishBboQuoteOperation, 'quote', beam.IndexedValue(
                nexus.BboQuote(bid, ask, timestamp), ticker)),
            (client.PublishBookQuoteOperation, 'quote', beam.IndexedValue(
                nexus.BookQuote('1234', True, venue, ask, timestamp), ticker)),
            (client.PublishTimeAndSaleOperation, 'time_and_sale',
                beam.IndexedValue(nexus.TimeAndSale(timestamp, price, size,
                    nexus.TimeAndSale.Condition(), 'CXA', '1234', '5678'),
                    ticker)),
            (client.PublishTickerStatusOperation, 'status',
                beam.IndexedValue(status, ticker))]
        for operation_type, field, value in cases:
            with self.subTest(operation=operation_type.__name__):
                result = self.executor.submit(self.client.publish, value)
                operation = self.pop_operation(operation_type)
                self.assertEqual(getattr(operation, field), value)
                operation.result.set()
                self.assertIsNone(result.result(timeout=5))

    def test_close(self):
        timestamp = datetime(2026, 9, 14, 10, 0)
        result = self.executor.submit(self.client.remove_order, 'order-123',
            timestamp)
        operation = self.pop_operation(
            nexus.tests.TestMarketDataFeedClient.RemoveOrderOperation)
        self.client.close()
        with self.assertRaises(beam.EndOfFileException):
            result.result(timeout=5)
        self.assertIsNone(operation.result.set())
        with self.assertRaises(beam.EndOfFileException):
            self.client.remove_order('order-456', timestamp)

    def test_result_exception(self):
        result = self.executor.submit(self.client.remove_order, 'order-123',
            datetime(2026, 9, 14, 10, 0))
        operation = self.pop_operation(
            nexus.tests.TestMarketDataFeedClient.RemoveOrderOperation)
        with self.assertRaises(TypeError):
            operation.result.set_exception('invalid order')
        self.assertFalse(result.done())
        service_result = operation.result
        del operation
        gc.collect()
        service_result.set_exception(ValueError('invalid order'))
        with self.assertRaises(ValueError):
            result.result(timeout=5)
        self.assertIsNone(service_result.set())

    def test_operations(self):
        client = nexus.MarketDataFeedClient(self.client)
        operation_types = nexus.tests.TestMarketDataFeedClient
        ticker = nexus.parse_ticker('BHP.ASX')
        info = nexus.TickerInfo(ticker, 'BHP Group', 'Materials',
            nexus.Quantity(100))
        price = nexus.Money.parse('42.75')
        size = nexus.Quantity(600)
        delta = nexus.Quantity(-100)
        timestamp = datetime(2026, 9, 14, 10, 0, 0, 123456)
        cases = [
            (client.add, operation_types.AddOperation, (info,), ('info',)),
            (client.modify_order_size, operation_types.ModifyOrderSizeOperation,
                ('order-123', size, timestamp), ('id', 'size', 'timestamp')),
            (client.offset_order_size, operation_types.OffsetOrderSizeOperation,
                ('order-123', delta, timestamp), ('id', 'delta', 'timestamp')),
            (client.modify_order_price,
                operation_types.ModifyOrderPriceOperation,
                ('order-123', price, timestamp), ('id', 'price', 'timestamp')),
            (client.remove_order, operation_types.RemoveOrderOperation,
                ('order-123', timestamp), ('id', 'timestamp'))]
        for method, operation_type, arguments, fields in cases:
            with self.subTest(operation=operation_type.__name__):
                result = self.executor.submit(method, *arguments)
                operation = self.pop_operation(operation_type)
                for field, value in zip(fields, arguments):
                    self.assertEqual(getattr(operation, field), value)
                operation.result.set()
                self.assertIsNone(result.result(timeout=5))


if __name__ == '__main__':
    unittest.main()
