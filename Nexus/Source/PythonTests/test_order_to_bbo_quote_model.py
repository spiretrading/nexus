from datetime import datetime
import unittest
import nexus


class TestOrderToBboQuoteModel(unittest.TestCase):
    def setUp(self):
        self.model = nexus.OrderToBboQuoteModel()
        self.timestamp = datetime(2026, 9, 25, 10, 0)

    def order(self, side, price, size):
        return nexus.BookQuote('MPID1', False, nexus.Venue('XTSE'),
            nexus.Quote(nexus.Money.parse(price), nexus.Quantity(size), side),
            self.timestamp)

    def test_default_bbo(self):
        self.assertEqual(self.model.bbo, nexus.BboQuote())

    def test_aggregate(self):
        for side in (nexus.Side.BID, nexus.Side.ASK):
            with self.subTest(side=side):
                self.model.clear(self.timestamp)
                order = self.order(side, '10', 100)
                self.assertTrue(self.model.add('1', order))
                order.mpid = 'MPID2'
                self.assertTrue(self.model.add('2', order))
                order.mpid = 'MPID1'
                order.venue = nexus.Venue('CHIC')
                self.assertTrue(self.model.add('3', order))
                if side == nexus.Side.BID:
                    quote = self.model.bbo.bid
                else:
                    quote = self.model.bbo.ask
                self.assertEqual(quote, nexus.Quote(nexus.Money.parse('10'),
                    nexus.Quantity(300), side))
                self.assertTrue(self.model.modify_size('1', nexus.Quantity(50),
                    self.timestamp))
                self.assertTrue(self.model.offset_size('2', nexus.Quantity(-25),
                    self.timestamp))
                self.assertTrue(self.model.remove('3', self.timestamp))
                if side == nexus.Side.BID:
                    quote = self.model.bbo.bid
                else:
                    quote = self.model.bbo.ask
                self.assertEqual(quote.size, nexus.Quantity(125))

    def test_price(self):
        self.model.add('1', self.order(nexus.Side.BID, '10', 100))
        previous = self.model.bbo
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        self.assertFalse(self.model.add('2',
            self.order(nexus.Side.BID, '9', 200)))
        self.assertEqual(self.model.bbo, previous)
        self.assertTrue(self.model.modify_price('2', nexus.Money.parse('11'),
            self.timestamp))
        self.assertEqual(self.model.bbo.bid,
            self.order(nexus.Side.BID, '11', 200).quote)
        self.assertEqual(self.model.bbo.timestamp, self.timestamp)
        self.assertTrue(self.model.remove('2', self.timestamp))
        self.assertEqual(self.model.bbo.bid, previous.bid)

    def test_replacement(self):
        self.model.add('1', self.order(nexus.Side.BID, '10', 100))
        previous = self.model.bbo
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        order = self.order(nexus.Side.BID, '10', 100)
        order.mpid = 'MPID2'
        order.venue = nexus.Venue('CHIC')
        self.assertFalse(self.model.add('1', order))
        self.assertEqual(self.model.bbo, previous)
        order.quote.side = nexus.Side.ASK
        self.assertTrue(self.model.add('1', order))
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(0))
        self.assertEqual(self.model.bbo.ask, order.quote)
        self.assertEqual(self.model.bbo.timestamp, self.timestamp)
        self.assertTrue(self.model.offset_size('1', nexus.Quantity(-200),
            self.timestamp))
        self.assertEqual(self.model.bbo.ask.size, nexus.Quantity(0))
        self.assertFalse(self.model.remove('1', self.timestamp))

    def test_unchanged_order(self):
        order = self.order(nexus.Side.ASK, '10', 100)
        self.model.add('1', order)
        previous = self.model.bbo
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        order.timestamp = self.timestamp
        self.assertFalse(self.model.add('1', order))
        self.assertFalse(self.model.modify_size('1', nexus.Quantity(100),
            self.timestamp))
        self.assertFalse(self.model.offset_size('1', nexus.Quantity(0),
            self.timestamp))
        self.assertFalse(self.model.modify_price('1', nexus.Money.parse('10'),
            self.timestamp))
        self.assertFalse(self.model.modify_size('2', nexus.Quantity(100),
            self.timestamp))
        self.assertFalse(self.model.offset_size('2', nexus.Quantity(100),
            self.timestamp))
        self.assertFalse(self.model.modify_price('2', nexus.Money.parse('10'),
            self.timestamp))
        self.assertFalse(self.model.remove('2', self.timestamp))
        self.assertEqual(self.model.bbo, previous)

    def test_clear(self):
        self.assertFalse(self.model.clear(self.timestamp))
        self.model.add('1', self.order(nexus.Side.BID, '10', 100))
        self.model.add('2', self.order(nexus.Side.ASK, '11', 200))
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        self.assertTrue(self.model.clear(self.timestamp))
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(0))
        self.assertEqual(self.model.bbo.ask.size, nexus.Quantity(0))
        self.assertEqual(self.model.bbo.timestamp, self.timestamp)
        previous = self.model.bbo
        self.timestamp = datetime(2026, 9, 25, 10, 2)
        self.assertFalse(self.model.clear(self.timestamp))
        self.assertEqual(self.model.bbo, previous)
        self.assertTrue(self.model.add('1',
            self.order(nexus.Side.ASK, '11', 50)))
        self.assertEqual(self.model.bbo.ask.size, nexus.Quantity(50))

    def test_bbo_snapshot(self):
        self.model.add('1', self.order(nexus.Side.BID, '10', 100))
        snapshot = self.model.bbo
        snapshot.bid.size = nexus.Quantity(500)
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(100))
        self.model.clear(self.timestamp)
        del self.model
        self.assertEqual(snapshot.bid.size, nexus.Quantity(500))


if __name__ == '__main__':
    unittest.main()
