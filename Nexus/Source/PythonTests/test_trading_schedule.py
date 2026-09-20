from datetime import datetime
import unittest
import nexus


class TestTradingSchedule(unittest.TestCase):
    def test_utc_events(self):
        for venue, timestamp, opening, expected in (
                ('XTSE', '2026-09-22 03:59:59', '1900-01-01 09:30:00',
                    '2026-09-21 13:30:00'),
                ('XTSE', '2026-09-22 04:00:00', '1900-01-01 09:30:00',
                    '2026-09-22 13:30:00'),
                ('XTSE', '2026-01-06 04:59:59', '1900-01-01 09:30:00',
                    '2026-01-05 14:30:00'),
                ('XASX', '2026-09-20 14:00:00', '1900-01-01 10:00:00',
                    '2026-09-21 00:00:00'),
                ('XASX', '2026-01-04 13:00:00', '1900-01-01 10:00:00',
                    '2026-01-04 23:00:00')):
            with self.subTest(venue=venue, timestamp=timestamp):
                venue = nexus.Venue(venue)
                timestamp = datetime.fromisoformat(timestamp)
                expected = datetime.fromisoformat(expected)
                opening_event = nexus.TradingSchedule.Event()
                opening_event.code = 'OPEN'
                opening_event.timestamp = datetime.fromisoformat(opening)
                rule = nexus.TradingSchedule.Rule()
                rule.venues = [venue]
                rule.events = [opening_event]
                schedule = nexus.TradingSchedule([rule])
                events = schedule.find(timestamp, venue)
                self.assertEqual(len(events), 1)
                self.assertEqual(events[0].code, 'OPEN')
                self.assertEqual(events[0].timestamp, expected)
                self.assertEqual(schedule.find(timestamp, venue,
                    lambda event: event.timestamp < expected), [])
                events = schedule.find(timestamp, venue,
                    lambda event: event.timestamp <= expected)
                self.assertEqual(len(events), 1)
                self.assertEqual(events[0].timestamp, expected)


if __name__ == '__main__':
    unittest.main()
