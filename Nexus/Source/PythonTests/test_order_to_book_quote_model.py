from datetime import datetime
import unittest
import nexus


class TestOrderToBookQuoteModel(unittest.TestCase):
    def setUp(self):
        self.model = nexus.OrderToBookQuoteModel()
        self.timestamp = datetime(2026, 9, 25, 10, 0)

    def order(self, size):
        return nexus.BookQuote('MPID1', False, nexus.Venue('XTSE'),
            nexus.Quote(nexus.Money.parse('10'), nexus.Quantity(size),
                nexus.Side.BID), self.timestamp)

    def test_aggregate(self):
        self.assertEqual(self.model.add('1', self.order(100)),
            [self.order(100)])
        updates = self.model.add('2', self.order(200))
        self.assertEqual(updates, [self.order(300)])
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        self.assertEqual(self.model.modify_size('1', nexus.Quantity(150),
            self.timestamp), [self.order(350)])
        self.assertEqual(self.model.offset_size('2', nexus.Quantity(-50),
            self.timestamp), [self.order(300)])
        self.assertEqual(self.model.remove('1', self.timestamp),
            [self.order(150)])
        self.assertEqual(self.model.remove('2', self.timestamp),
            [self.order(0)])
        self.assertEqual(self.model.clear(self.timestamp), [])

    def test_replace(self):
        self.model.add('1', self.order(100))
        self.model.add('2', self.order(200))
        self.assertEqual(self.model.add('1', self.order(150)),
            [self.order(350)])
        replacement = self.order(50)
        replacement.venue = nexus.Venue('CHIC')
        replacement.mpid = 'MPID2'
        replacement.quote.side = nexus.Side.ASK
        replacement.quote.price = nexus.Money.parse('11')
        self.assertEqual(self.model.add('1', replacement),
            [self.order(200), replacement])
        replacement.quote.size = nexus.Quantity(0)
        self.assertEqual(self.model.remove('1', self.timestamp), [replacement])

    def test_venues(self):
        self.model.add('1', self.order(100))
        second = self.order(200)
        second.venue = nexus.Venue('CHIC')
        self.assertEqual(self.model.add('2', second), [second])
        self.assertEqual(self.model.remove('1', self.timestamp),
            [self.order(0)])
        second.quote.size = nexus.Quantity(300)
        self.assertEqual(self.model.modify_size('2', nexus.Quantity(300),
            self.timestamp), [second])

    def test_modify_price(self):
        self.model.add('1', self.order(100))
        destination = self.order(200)
        destination.quote.price = nexus.Money.parse('11')
        self.model.add('2', destination)
        destination.quote.size = nexus.Quantity(300)
        self.assertEqual(self.model.modify_price('1', nexus.Money.parse('11'),
            self.timestamp), [self.order(0), destination])
        self.assertEqual(self.model.modify_price('1', nexus.Money.parse('11'),
            self.timestamp), [])

    def test_clear(self):
        for i in range(4):
            order = self.order(100)
            order.quote.price = nexus.Money.parse(str(10 + i))
            self.model.add(str(i), order)
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        updates = self.model.clear(self.timestamp)
        self.assertEqual(len(updates), 4)
        for update in updates:
            self.assertEqual(update.quote.size, nexus.Quantity(0))
            self.assertEqual(update.timestamp, self.timestamp)
        self.assertEqual(self.model.clear(self.timestamp), [])
        self.assertEqual(self.model.add('1', self.order(50)), [self.order(50)])
        self.assertEqual(updates[0].quote.size, nexus.Quantity(0))

    def test_absent_and_unchanged(self):
        self.assertEqual(self.model.modify_size('1', nexus.Quantity(100),
            self.timestamp), [])
        self.assertEqual(self.model.offset_size('1', nexus.Quantity(100),
            self.timestamp), [])
        self.assertEqual(self.model.modify_price('1', nexus.Money.parse('10'),
            self.timestamp), [])
        self.assertEqual(self.model.remove('1', self.timestamp), [])
        self.model.add('1', self.order(100))
        self.assertEqual(self.model.add('1', self.order(100)), [])
        self.assertEqual(self.model.modify_size('1', nexus.Quantity(100),
            self.timestamp), [])
        self.assertEqual(self.model.offset_size('1', nexus.Quantity(0),
            self.timestamp), [])
        self.assertEqual(self.model.offset_size('1', nexus.Quantity(-150),
            self.timestamp), [self.order(0)])
        self.assertEqual(self.model.remove('1', self.timestamp), [])

    def test_bbo_composition(self):
        bbo = nexus.BookQuoteToBboQuoteModel()
        for quote in self.model.add('1', self.order(100)):
            bbo.update(quote)
        for quote in self.model.add('2', self.order(200)):
            bbo.update(quote)
        self.assertEqual(bbo.bbo.bid.size, nexus.Quantity(300))
        for quote in self.model.clear(self.timestamp):
            bbo.update(quote)
        self.assertEqual(bbo.bbo.bid.size, nexus.Quantity(0))


if __name__ == '__main__':
    unittest.main()
