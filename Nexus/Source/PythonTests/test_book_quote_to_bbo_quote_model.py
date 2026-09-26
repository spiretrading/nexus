from datetime import datetime
import unittest
import nexus


class TestBookQuoteToBboQuoteModel(unittest.TestCase):
    def setUp(self):
        self.model = nexus.BookQuoteToBboQuoteModel()
        self.timestamp = datetime(2026, 9, 25, 10, 0)

    def quote(self, side, price, size, mpid='AU000'):
        return nexus.BookQuote(mpid, False, nexus.venues.ASX,
            nexus.Quote(nexus.Money.parse(price), nexus.Quantity(size), side),
            self.timestamp)

    def test_default_bbo(self):
        self.assertEqual(self.model.bbo, nexus.BboQuote())

    def test_update(self):
        bid = self.quote(nexus.Side.BID, '10.50', 100)
        ask = self.quote(nexus.Side.ASK, '10.60', 200)
        self.assertTrue(self.model.update(bid))
        self.assertTrue(self.model.update(ask))
        self.assertEqual(self.model.bbo,
            nexus.BboQuote(bid.quote, ask.quote, self.timestamp))
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        self.assertFalse(self.model.update(
            self.quote(nexus.Side.BID, '10.40', 300)))
        self.assertFalse(self.model.update(ask))
        self.assertEqual(self.model.bbo.timestamp, bid.timestamp)

    def test_mpid_size(self):
        self.model.update(self.quote(nexus.Side.BID, '10.50', 100, 'AU100'))
        self.model.update(self.quote(nexus.Side.BID, '10.50', 200, 'AU200'))
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(300))
        self.assertTrue(self.model.update(
            self.quote(nexus.Side.BID, '10.50', 300, 'AU100')))
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(500))
        previous = self.model.bbo
        self.timestamp = datetime(2026, 9, 25, 10, 1)
        self.assertFalse(self.model.update(
            self.quote(nexus.Side.BID, '10.50', 200, 'AU200')))
        self.assertEqual(self.model.bbo, previous)
        self.assertTrue(self.model.update(
            self.quote(nexus.Side.BID, '10.50', 50, 'AU200')))
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(350))

    def test_delete(self):
        for side in (nexus.Side.BID, nexus.Side.ASK):
            with self.subTest(side=side):
                self.model.update(self.quote(side, '10.50', 100))
                self.assertTrue(self.model.update(self.quote(side, '10.50', 0)))
                bbo = self.model.bbo
                self.assertEqual(bbo.bid.size, nexus.Quantity(0))
                self.assertEqual(bbo.ask.size, nexus.Quantity(0))
                self.assertFalse(self.model.update(
                    self.quote(side, '10.50', 0)))

    def test_bbo_snapshot(self):
        bid = self.quote(nexus.Side.BID, '10.50', 100)
        self.model.update(bid)
        snapshot = self.model.bbo
        snapshot.bid.size = nexus.Quantity(500)
        self.assertEqual(self.model.bbo.bid.size, nexus.Quantity(100))
        self.model.update(self.quote(nexus.Side.BID, '10.50', 200))
        self.assertEqual(snapshot.bid.size, nexus.Quantity(500))
        del self.model
        self.assertEqual(snapshot.bid.price, bid.quote.price)


if __name__ == '__main__':
    unittest.main()
