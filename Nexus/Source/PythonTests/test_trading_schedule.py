from datetime import date, datetime
import unittest
import beam
import nexus


class TestTradingSchedule(unittest.TestCase):
    def setUp(self):
        self.opening = nexus.TradingSchedule.Event()
        self.opening.code = 'OPEN'
        self.opening.timestamp = datetime(1900, 1, 1, 9, 30)
        self.closing = nexus.TradingSchedule.Event()
        self.closing.code = 'CLOSE'
        self.closing.timestamp = datetime(1900, 1, 1, 16)
        self.rule = nexus.TradingSchedule.Rule()
        self.rule.venues = [nexus.venues.TSX]
        self.rule.events = [self.opening, self.closing]

    def test_event(self):
        empty = nexus.TradingSchedule.Event()
        self.assertEqual(empty.code, '')
        self.assertIsNone(empty.timestamp)
        for field, value in (
                ('code', 'CLOSE'),
                ('timestamp', datetime(1900, 1, 1, 16, 0, 0, 123456))):
            with self.subTest(field=field):
                event = nexus.TradingSchedule.Event(self.opening)
                self.assertEqual(event, self.opening)
                setattr(event, field, value)
                self.assertEqual(getattr(event, field), value)
                self.assertNotEqual(event, self.opening)
        self.assertEqual(str(self.opening), '(OPEN 1900-Jan-01 09:30:00)')

    def test_rule(self):
        empty = nexus.TradingSchedule.Rule()
        for field in (
                'venues', 'weekdays', 'days', 'months', 'years', 'events'):
            self.assertEqual(getattr(empty, field), [])
        for field, value in (
                ('venues', [nexus.venues.ASX]), ('events', [self.closing])):
            with self.subTest(field=field):
                rule = nexus.TradingSchedule.Rule(self.rule)
                self.assertEqual(rule, self.rule)
                setattr(rule, field, value)
                self.assertEqual(getattr(rule, field), value)
                self.assertNotEqual(rule, self.rule)
        self.opening.code = 'changed'
        self.assertEqual(self.rule.events[0].code, 'OPEN')
        events = self.rule.events
        events[0].code = 'changed'
        self.assertEqual(self.rule.events[0].code, 'changed')
        rule = nexus.TradingSchedule.Rule(self.rule)
        rule.events[0].code = 'copy'
        self.assertEqual(self.rule.events[0].code, 'changed')

    def test_find(self):
        timestamp = datetime(2026, 9, 21, 15)
        self.assertEqual(
            nexus.TradingSchedule([]).find(timestamp, nexus.venues.TSX), [])
        schedule = nexus.TradingSchedule([self.rule])
        expected = []
        for event, hour, minute in (
                (self.opening, 13, 30), (self.closing, 20, 0)):
            event = nexus.TradingSchedule.Event(event)
            event.timestamp = datetime(2026, 9, 21, hour, minute)
            expected.append(event)
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX), expected)
        self.assertEqual(schedule.find(timestamp, nexus.venues.ASX), [])
        self.rule.events = []
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX), expected)
        events = schedule.find(timestamp, nexus.venues.TSX)
        events[0].code = 'changed'
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX), expected)

    def test_rule_precedence(self):
        timestamp = datetime(2026, 9, 21, 15)
        closure = nexus.TradingSchedule.Rule()
        closure.venues = [nexus.venues.TSX]
        schedule = nexus.TradingSchedule([closure, self.rule])
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX), [])
        wildcard = nexus.TradingSchedule.Rule(self.rule)
        wildcard.venues = []
        schedule = nexus.TradingSchedule([self.rule, wildcard])
        events = schedule.find(timestamp, nexus.venues.ASX)
        self.assertEqual([event.code for event in events], ['OPEN', 'CLOSE'])
        self.assertEqual(events[0].timestamp, datetime(2026, 9, 21, 23, 30))

    def test_predicate(self):
        schedule = nexus.TradingSchedule([self.rule])
        timestamp = datetime(2026, 9, 21, 15)
        observed = []

        def predicate(event):
            observed.append(event)
            return event.code == 'CLOSE'

        events = schedule.find(timestamp, nexus.venues.TSX, predicate)
        self.assertEqual([event.code for event in observed], ['OPEN', 'CLOSE'])
        self.assertEqual(observed[0].timestamp, datetime(2026, 9, 21, 13, 30))
        self.assertEqual(events, [observed[1]])
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX,
            lambda event: False), [])

        def fail(event):
            raise ValueError('predicate failure')

        with self.assertRaises(ValueError):
            schedule.find(timestamp, nexus.venues.TSX, fail)
        self.assertEqual(schedule.find(timestamp, nexus.venues.TSX), observed)

    def test_date_filters(self):
        for field, value in (
                ('weekdays', [1, 5]), ('days', [1, 21]),
                ('months', [1, 9]), ('years', [2026, 2027])):
            with self.subTest(field=field):
                rule = nexus.TradingSchedule.Rule(self.rule)
                setattr(rule, field, value)
                self.assertEqual(getattr(rule, field), value)
                self.assertNotEqual(rule, self.rule)
                self.assertEqual(
                    getattr(nexus.TradingSchedule.Rule(rule), field), value)
                schedule = nexus.TradingSchedule([rule])
                self.assertEqual(len(schedule.find(
                    datetime(2026, 9, 21, 15), nexus.venues.TSX)), 2)
                setattr(rule, field, [])
                self.assertEqual(getattr(rule, field), [])

    def test_date_filter_ranges(self):
        for field, bounds, invalid in (
                ('weekdays', [0, 6], [7]), ('days', [1, 31], [0, 32]),
                ('months', [1, 12], [0, 13]),
                ('years', [1400, 9999], [1399, 10000])):
            with self.subTest(field=field):
                rule = nexus.TradingSchedule.Rule()
                setattr(rule, field, bounds)
                self.assertEqual(getattr(rule, field), bounds)
                for value in invalid:
                    with self.assertRaises(IndexError):
                        setattr(rule, field, [bounds[0], value])
                    self.assertEqual(getattr(rule, field), bounds)
                for value in (-1, 65536, None, 'invalid'):
                    with self.assertRaises(TypeError):
                        setattr(rule, field, [bounds[0], value])
                    self.assertEqual(getattr(rule, field), bounds)

    def test_is_match(self):
        rule = nexus.TradingSchedule.Rule(self.rule)
        self.assertTrue(nexus.is_match(nexus.venues.TSX,
            date(2026, 9, 21), rule))
        self.assertFalse(nexus.is_match(nexus.venues.ASX,
            date(2026, 9, 21), rule))
        for field, matching, other in (
                ('weekdays', 1, 2), ('days', 21, 22), ('months', 9, 10),
                ('years', 2026, 2027)):
            with self.subTest(field=field):
                rule = nexus.TradingSchedule.Rule(self.rule)
                setattr(rule, field, [matching])
                self.assertTrue(nexus.is_match(nexus.venues.TSX,
                    date(2026, 9, 21), rule))
                setattr(rule, field, [other])
                self.assertFalse(nexus.is_match(nexus.venues.TSX,
                    date(2026, 9, 21), rule))
        self.assertTrue(nexus.is_match(nexus.venues.ASX,
            date(2026, 9, 21), nexus.TradingSchedule.Rule()))
        self.assertTrue(nexus.is_match(nexus.venues.TSX,
            datetime(2026, 9, 21, 15), self.rule))
        with self.assertRaises(TypeError):
            nexus.is_match(nexus.venues.TSX, '2026-09-21', self.rule)
        with self.assertRaises(IndexError):
            nexus.is_match(nexus.venues.TSX, date(1399, 12, 31), self.rule)

    def test_parse(self):
        source = '''
- venues: [TSX]
  dates: [2026-09-21, 2026-09-23]
- venues: [TSX]
  time:
    weekdays: [Mon, Tue, Wed, Thu, Fri]
    days: [21, 22, 23]
    months: [Sep]
    years: [2026]
  events:
    - code: CLOSE
      time: 16:00:00
    - code: OPEN
      time: 09:30:00
'''
        database = nexus.VenueDatabase()
        entry = nexus.VenueDatabase.Entry(nexus.VENUES.select(nexus.venues.TSX))
        entry.display_name = 'Test'
        database.add(entry)
        for source, arguments in (
                (source, ()), (source.replace('TSX', 'Test'), (database,))):
            with self.subTest(arguments=arguments):
                schedule = nexus.parse_trading_schedule(
                    beam.parse_yaml(source), *arguments)
                for day in (21, 23, 24):
                    self.assertEqual(schedule.find(
                        datetime(2026, 9, day, 15), nexus.venues.TSX), [])
                events = schedule.find(
                    datetime(2026, 9, 22, 15), nexus.venues.TSX)
                self.assertEqual([event.code for event in events],
                    ['OPEN', 'CLOSE'])
                self.assertEqual([event.timestamp for event in events],
                    [datetime(2026, 9, 22, 13, 30), datetime(2026, 9, 22, 20)])
                self.assertEqual(schedule.find(
                    datetime(2026, 9, 22, 15), nexus.venues.ASX), [])
        schedule = nexus.parse_trading_schedule(beam.parse_yaml('[]'))
        self.assertEqual(schedule.find(
            datetime(2026, 9, 22, 15), nexus.venues.TSX), [])
        with self.assertRaises(RuntimeError):
            nexus.parse_trading_schedule(beam.parse_yaml(
                '- events: [{code: OPEN, time: invalid}]'))

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
