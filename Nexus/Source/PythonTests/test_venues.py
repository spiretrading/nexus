import unittest
import nexus


class TestVenues(unittest.TestCase):
    def test_canadian_market_centers(self):
        for center, mic, name in (
                ('ALX', 'XATX', 'ALX'), ('ALD', 'ADRK', 'ALD'),
                ('ICX', 'XICX', 'ICX'), ('LIQ', 'LICA', 'LIQ'),
                ('AQN', 'NEON', 'NEON')):
            with self.subTest(center=center):
                venue = nexus.Venue(mic)
                self.assertEqual(getattr(nexus.venues, name), venue)
                self.assertEqual(nexus.parse_venue(name), venue)
                entry = nexus.from_market_center(center)
                self.assertEqual(entry.venue, venue)
                self.assertEqual(entry.display_name, name)
                self.assertEqual(entry.time_zone, 'America/Toronto')


if __name__ == '__main__':
    unittest.main()
