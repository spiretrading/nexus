from datetime import datetime
import gc
import unittest
import beam
import nexus


class TestTimeAndSale(unittest.TestCase):
    def setUp(self):
        self.condition = nexus.TimeAndSale.Condition()
        self.condition.type = nexus.TimeAndSale.Type.CORRECTION
        self.condition.code = 'U;CrossType=VWAP'
        self.trade = nexus.TimeAndSale(
            datetime(2026, 9, 21, 14, 30, 12, 123456),
            nexus.Money.parse('23.125'), nexus.Quantity(125.5),
            self.condition, 'TSX', '001', '099')

    def test_construction(self):
        default = nexus.TimeAndSale()
        self.assertIsNone(default.timestamp)
        self.assertEqual(default.price, nexus.Money())
        self.assertEqual(default.size, nexus.Quantity())
        self.assertEqual(default.condition.type, nexus.TimeAndSale.Type.NONE)
        self.assertEqual(default.condition.code, '')
        self.assertEqual(default.market_center, '')
        self.assertEqual(default.buyer_mpid, '')
        self.assertEqual(default.seller_mpid, '')
        self.assertEqual(self.trade.timestamp,
            datetime(2026, 9, 21, 14, 30, 12, 123456))
        self.assertEqual(self.trade.price, nexus.Money.parse('23.125'))
        self.assertEqual(self.trade.size, nexus.Quantity(125.5))
        self.assertEqual(self.trade.condition, self.condition)
        self.assertEqual(self.trade.market_center, 'TSX')
        self.assertEqual(self.trade.buyer_mpid, '001')
        self.assertEqual(self.trade.seller_mpid, '099')
        self.condition.code = 'changed'
        self.assertEqual(self.trade.condition.code, 'U;CrossType=VWAP')

    def test_fields_and_copy(self):
        condition = nexus.TimeAndSale.Condition()
        condition.type = nexus.TimeAndSale.Type.CANCELLATION
        condition.code = 'X'
        for field, value in (
                ('timestamp', datetime(2026, 12, 21, 15, 0, 0, 654321)),
                ('price', nexus.Money.parse('99.75')),
                ('size', nexus.Quantity(200.25)), ('condition', condition),
                ('market_center', 'NEOL'), ('buyer_mpid', '002'),
                ('seller_mpid', '098')):
            with self.subTest(field=field):
                trade = nexus.TimeAndSale(self.trade)
                self.assertEqual(trade, self.trade)
                setattr(trade, field, value)
                self.assertEqual(getattr(trade, field), value)
                self.assertNotEqual(trade, self.trade)
        trade = nexus.TimeAndSale(self.trade)
        trade.condition.code = 'changed'
        self.assertEqual(self.trade.condition.code, 'U;CrossType=VWAP')
        trade.condition = condition
        condition.code = 'changed'
        self.assertEqual(trade.condition.code, 'X')

    def test_condition_copy(self):
        default = nexus.TimeAndSale.Condition()
        self.assertEqual(default.type, nexus.TimeAndSale.Type.NONE)
        self.assertEqual(default.code, '')
        for field, value in (
                ('type', nexus.TimeAndSale.Type.CANCELLATION), ('code', 'X')):
            with self.subTest(field=field):
                condition = nexus.TimeAndSale.Condition(self.condition)
                self.assertEqual(condition, self.condition)
                setattr(condition, field, value)
                self.assertEqual(getattr(condition, field), value)
                self.assertNotEqual(condition, self.condition)

    def test_condition_lifetime(self):
        trade = nexus.TimeAndSale(self.trade)
        condition = trade.condition
        del trade
        gc.collect()
        self.assertEqual(condition, self.condition)
        condition.type = nexus.TimeAndSale.Type.CANCELLATION
        condition.code = 'X'
        self.assertEqual(str(condition), '(CANCELLATION X)')

    def test_string(self):
        self.assertEqual(str(self.trade),
            '(2026-Sep-21 14:30:12.123456 23.125 125.500000 '
            '(CORRECTION U;CrossType=VWAP) TSX 001 099)')

    def test_queues(self):
        for prefix in ('TimeAndSale', 'SequencedTimeAndSale'):
            with self.subTest(prefix=prefix):
                queue = getattr(nexus, prefix + 'Queue')()
                self.addCleanup(queue.close)
                self.assertIsNone(queue.try_pop())
                trade = nexus.TimeAndSale(self.trade)
                expected = nexus.TimeAndSale(trade)
                value = trade
                if prefix == 'SequencedTimeAndSale':
                    value = beam.SequencedValue(trade, beam.Sequence(42))
                    expected = beam.SequencedValue(expected, beam.Sequence(42))
                queue.push(value)
                trade.market_center = 'changed'
                self.assertEqual(queue.pop(), expected)
                self.assertIsNone(queue.try_pop())
                source = beam.Queue()
                self.addCleanup(source.close)
                writer = getattr(nexus, prefix + 'QueueWriter')(source)
                reader = getattr(nexus, prefix + 'QueueReader')(source)
                writer.push(expected)
                self.assertEqual(reader.pop(), expected)
                self.assertIsNone(reader.try_pop())

    def test_publishers(self):
        for prefix in ('TimeAndSale', 'SequencedTimeAndSale'):
            with self.subTest(prefix=prefix):
                publisher = getattr(nexus, prefix + 'QueueWriterPublisher')()
                self.addCleanup(publisher.close)
                queue = getattr(nexus, prefix + 'Queue')()
                self.addCleanup(queue.close)
                subscriber = beam.Queue()
                self.addCleanup(subscriber.close)
                publisher.monitor(queue)
                publisher.monitor(subscriber)
                value = self.trade
                if prefix == 'SequencedTimeAndSale':
                    value = beam.SequencedValue(value, beam.Sequence(42))
                publisher.push(value)
                self.assertEqual(queue.pop(), value)
                self.assertEqual(subscriber.pop(), value)
                self.assertIsNone(queue.try_pop())
                self.assertIsNone(subscriber.try_pop())

    def test_conditions(self):
        for name, code, value, label in (
                ('NONE', '', -1, 'NONE'),
                ('REGULAR', '@', 0, '@'), ('OPEN', 'O', 1, 'OPEN'),
                ('CLOSE', 'C', 2, 'CLOSE'), ('REOPEN', 'R', 3, 'REOPEN'),
                ('AUCTION', 'A', 4, 'AUCTION'),
                ('CORRECTION', 'U', 5, 'CORRECTION'),
                ('CANCELLATION', 'X', 6, 'CANCELLATION')):
            with self.subTest(name=name):
                condition_type = getattr(nexus.TimeAndSale.Type, name)
                self.assertEqual(int(condition_type), value)
                condition = nexus.TimeAndSale.Condition()
                condition.type = condition_type
                condition.code = code
                self.assertEqual(condition.type, condition_type)
                self.assertEqual(str(condition), f'({label} {code})')
                trade = nexus.TimeAndSale()
                trade.condition = condition
                self.assertEqual(trade.condition, condition)


if __name__ == '__main__':
    unittest.main()
