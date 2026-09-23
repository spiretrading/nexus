from datetime import datetime
import gc
import unittest
import beam
import nexus


class TestVenues(unittest.TestCase):
    def test_venue(self):
        empty = nexus.Venue()
        self.assertFalse(empty)
        self.assertEqual(empty.code, '')
        self.assertEqual(str(empty), '')
        venue = nexus.Venue('XTSE')
        self.assertTrue(venue)
        self.assertEqual(venue.code, 'XTSE')
        self.assertEqual(str(venue), 'TSX')
        self.assertEqual(nexus.Venue(venue), venue)
        self.assertNotEqual(venue, nexus.Venue('XASX'))
        self.assertLess(nexus.Venue('XASX'), venue)
        self.assertLessEqual(venue, nexus.Venue(venue))
        self.assertGreater(venue, nexus.Venue('XASX'))
        self.assertGreaterEqual(venue, nexus.Venue(venue))
        self.assertEqual(str(nexus.Venue('TEST')), 'TEST')
        with self.assertRaises(AttributeError):
            venue.code = 'XASX'

    def test_entry(self):
        empty = nexus.VenueDatabase.Entry()
        self.assertEqual(empty.venue, nexus.Venue())
        self.assertEqual(empty.country_code, nexus.CountryCode())
        self.assertEqual(empty.currency, nexus.CurrencyId())
        for field in ('market_center', 'time_zone', 'description',
                'display_name'):
            self.assertEqual(getattr(empty, field), '')
        entry = nexus.VenueDatabase.Entry()
        for field, value in (
                ('venue', nexus.Venue('TEST')),
                ('country_code', nexus.countries.CA),
                ('currency', nexus.currencies.CAD), ('market_center', 'TST'),
                ('time_zone', 'America/Toronto'),
                ('description', 'Test Exchange'), ('display_name', 'Test')):
            with self.subTest(field=field):
                copy = nexus.VenueDatabase.Entry(entry)
                self.assertEqual(copy, entry)
                setattr(entry, field, value)
                self.assertEqual(getattr(entry, field), value)
                self.assertNotEqual(copy, entry)
        self.assertEqual(str(entry),
            '(TEST CA TST America/Toronto CAD Test Exchange Test)')

    def test_database(self):
        database = nexus.VenueDatabase()
        self.assertEqual(list(database.entries), [])
        entries = [
            nexus.VENUES.select(nexus.venues.TSX),
            nexus.VENUES.select(nexus.venues.ASX),
            nexus.VENUES.select(nexus.venues.TSXV)]
        for entry in entries:
            database.add(entry)
        self.assertCountEqual(list(database.entries), entries)
        for entry in entries:
            with self.subTest(venue=entry.venue.code):
                self.assertEqual(database.select(entry.venue), entry)
                self.assertEqual(database.select(entry.venue.code), entry)
                self.assertEqual(
                    database.select_display_name(entry.display_name), entry)
        self.assertCountEqual(database.select(nexus.countries.CA),
            [entries[0], entries[2]])
        self.assertEqual(database.select(nexus.countries.US), [])
        self.assertEqual(database.select(nexus.Venue('TEST')),
            nexus.VenueDatabase.Entry())
        self.assertEqual(database.select('TEST'), nexus.VenueDatabase.Entry())
        self.assertEqual(database.select_display_name('missing'),
            nexus.VenueDatabase.Entry())
        copy = nexus.VenueDatabase(database)
        database.remove(nexus.venues.TSX)
        self.assertEqual(copy.select(nexus.venues.TSX), entries[0])
        self.assertEqual(database.select(nexus.venues.TSX),
            nexus.VenueDatabase.Entry())
        database.remove(nexus.venues.TSX)
        self.assertCountEqual(list(database.entries), entries[1:])

    def test_lookups(self):
        database = nexus.VenueDatabase()
        entry = nexus.VenueDatabase.Entry(nexus.VENUES.select(nexus.venues.TSX))
        entry.display_name = 'Test'
        entry.market_center = 'TST'
        database.add(entry)
        for name in ('Test', 'XTSE'):
            self.assertEqual(nexus.parse_venue(name, database), entry.venue)
            self.assertEqual(nexus.parse_venue_entry(name, database), entry)
        self.assertEqual(nexus.from_market_center('TST', database), entry)
        for name in ('TSX', 'XTSE'):
            self.assertEqual(nexus.parse_venue(name), nexus.venues.TSX)
            self.assertEqual(nexus.parse_venue_entry(name),
                nexus.VENUES.select(nexus.venues.TSX))
        self.assertEqual(nexus.from_market_center('TSE'),
            nexus.VENUES.select(nexus.venues.TSX))
        for arguments in ((), (database,)):
            self.assertEqual(
                nexus.parse_venue('MISS', *arguments), nexus.Venue())
            self.assertEqual(nexus.parse_venue_entry('MISS', *arguments),
                nexus.VenueDatabase.Entry())
            self.assertEqual(nexus.from_market_center('MISS', *arguments),
                nexus.VenueDatabase.Entry())

    def test_time_conversion(self):
        for venue, utc, local, start, end in (
                (nexus.venues.TSX, datetime(2026, 9, 22, 3, 59, 59, 123456),
                    datetime(2026, 9, 21, 23, 59, 59, 123456),
                    datetime(2026, 9, 21, 4), datetime(2026, 9, 22, 4)),
                (nexus.venues.TSX, datetime(2026, 1, 5, 14, 30),
                    datetime(2026, 1, 5, 9, 30), datetime(2026, 1, 5, 5),
                    datetime(2026, 1, 6, 5)),
                (nexus.venues.ASX, datetime(2026, 1, 4, 23),
                    datetime(2026, 1, 5, 10), datetime(2026, 1, 4, 13),
                    datetime(2026, 1, 5, 13)),
                (nexus.venues.ASX, datetime(2026, 9, 21),
                    datetime(2026, 9, 21, 10), datetime(2026, 9, 20, 14),
                    datetime(2026, 9, 21, 14))):
            with self.subTest(venue=venue.code, utc=utc):
                self.assertEqual(nexus.utc_to_venue(venue, utc), local)
                self.assertEqual(nexus.venue_to_utc(venue, local), utc)
                self.assertEqual(nexus.utc_start_of_day(venue, utc), start)
                self.assertEqual(nexus.utc_end_of_day(venue, utc), end)
        for convert in (nexus.utc_to_venue, nexus.venue_to_utc,
                nexus.utc_start_of_day, nexus.utc_end_of_day):
            self.assertIsNone(convert(nexus.venues.TSX, None))
        self.assertIsNone(nexus.utc_to_venue(nexus.Venue('MISS'),
            datetime(2026, 9, 21)))
        self.assertIsNone(nexus.venue_to_utc(nexus.Venue('MISS'),
            datetime(2026, 9, 21)))

    def test_set_venues(self):
        original = nexus.VenueDatabase(nexus.VENUES)
        self.addCleanup(nexus.set_venues, original)
        database = nexus.VenueDatabase()
        entry = nexus.VenueDatabase.Entry(original.select(nexus.venues.TSX))
        entry.display_name = 'Test'
        entry.market_center = 'TST'
        database.add(entry)
        nexus.set_venues(database)
        self.assertEqual(nexus.parse_venue('Test'), nexus.venues.TSX)
        self.assertEqual(nexus.from_market_center('TST'), entry)
        self.assertEqual(str(nexus.venues.TSX), 'Test')
        self.assertEqual(list(nexus.VENUES.entries), [entry])
        database.remove(nexus.venues.TSX)
        self.assertEqual(nexus.parse_venue_entry('Test'), entry)

    def test_database_lifetime(self):
        database = nexus.VenueDatabase()
        expected = nexus.VenueDatabase.Entry(
            nexus.VENUES.select(nexus.venues.TSX))
        database.add(expected)
        entries = database.entries
        selected = database.select(nexus.venues.TSX)
        del database
        gc.collect()
        self.assertEqual(list(entries), [expected])
        self.assertEqual(selected, expected)

    def test_parse_database(self):
        source = ('{venue: TEST, country_code: CA, time_zone: America/Toronto, '
            'currency: CAD, description: Test Exchange, display_name: Test}')
        countries = nexus.CountryDatabase(nexus.COUNTRIES)
        currencies = nexus.CurrencyDatabase(nexus.CURRENCIES)
        for arguments in ((), (countries, currencies)):
            with self.subTest(arguments=arguments):
                entry = nexus.parse_venue_database_entry(
                    beam.parse_yaml(source), *arguments)
                self.assertEqual(entry.venue, nexus.Venue('TEST'))
                self.assertEqual(entry.country_code, nexus.countries.CA)
                self.assertEqual(entry.currency, nexus.currencies.CAD)
                self.assertEqual(entry.time_zone, 'America/Toronto')
                self.assertEqual(entry.description, 'Test Exchange')
                self.assertEqual(entry.display_name, 'Test')
                self.assertEqual(entry.market_center, 'Test')
                database = nexus.parse_venue_database(
                    beam.parse_yaml('[' + source + ']'), *arguments)
                self.assertEqual(list(database.entries), [entry])
                self.assertEqual(list(nexus.parse_venue_database(
                    beam.parse_yaml('[]'), *arguments).entries), [])
        source = source.replace(
            'venue: TEST', 'market_center: TST, venue: TEST')
        self.assertEqual(nexus.parse_venue_database_entry(
            beam.parse_yaml(source)).market_center, 'TST')
        for parse, text in (
                (nexus.parse_venue_database_entry, source),
                (nexus.parse_venue_database, '[' + source + ']')):
            for arguments in (
                    (nexus.CountryDatabase(), currencies),
                    (countries, nexus.CurrencyDatabase())):
                with self.assertRaises(RuntimeError):
                    parse(beam.parse_yaml(text), *arguments)
        for parse, source in (
                (nexus.parse_venue_database_entry, '{}'),
                (nexus.parse_venue_database, '[{}]')):
            with self.assertRaises(RuntimeError):
                parse(beam.parse_yaml(source))
        with self.assertRaises(RuntimeError):
            beam.parse_yaml('[')

    def test_time_zone_databases(self):
        utc = datetime(2026, 9, 21, 13, 30)
        for arguments in (
                (nexus.TIME_ZONES,), (nexus.VENUES, nexus.TIME_ZONES)):
            self.assertEqual(nexus.utc_to_venue(nexus.venues.TSX, utc,
                *arguments), datetime(2026, 9, 21, 9, 30))
            self.assertEqual(nexus.venue_to_utc(nexus.venues.TSX,
                datetime(2026, 9, 21, 9, 30), *arguments), utc)
            self.assertEqual(nexus.utc_start_of_day(nexus.venues.TSX, utc,
                *arguments), datetime(2026, 9, 21, 4))
            self.assertEqual(nexus.utc_end_of_day(nexus.venues.TSX, utc,
                *arguments), datetime(2026, 9, 22, 4))
        database = nexus.VenueDatabase()
        entry = nexus.VenueDatabase.Entry()
        entry.venue = nexus.Venue('TEST')
        entry.time_zone = 'Test/Zone'
        database.add(entry)
        zones = beam.TimeZoneDatabase(
            '"Test/Zone","TST","Test","","","+03:00:00",'
            '"+00:00:00","","","",""\n')
        for zones in (zones, beam.TimeZoneDatabase(zones)):
            self.assertEqual(nexus.utc_to_venue(entry.venue, utc, database,
                zones), datetime(2026, 9, 21, 16, 30))
            self.assertEqual(nexus.venue_to_utc(entry.venue,
                datetime(2026, 9, 21, 16, 30), database, zones), utc)
            self.assertEqual(nexus.utc_start_of_day(entry.venue, utc, database,
                zones), datetime(2026, 9, 20, 21))
            self.assertEqual(nexus.utc_end_of_day(entry.venue, utc, database,
                zones), datetime(2026, 9, 21, 21))
        self.assertIsNone(nexus.utc_to_venue(entry.venue, utc, database,
            beam.TimeZoneDatabase()))
        with self.assertRaises(IndexError):
            beam.TimeZoneDatabase('invalid\n')

    def test_constants(self):
        for name, mic in (
                ('ALD', 'ADRK'), ('ALX', 'XATX'), ('ASX', 'XASX'),
                ('CHIC', 'CHIC'), ('CSE', 'XCNQ'), ('CSE2', 'CSE2'),
                ('CXA', 'CHIA'), ('CXD', 'XCXD'), ('ICX', 'XICX'),
                ('LIQ', 'LICA'), ('LYNX', 'LYNX'), ('MATN', 'MATN'),
                ('NEOE', 'NEOE'), ('NEON', 'NEON'), ('OMGA', 'OMGA'),
                ('PURE', 'PURE'), ('TSX', 'XTSE'), ('TSXV', 'XTSX'),
                ('XATS', 'XATS'), ('XCX2', 'XCX2')):
            with self.subTest(name=name):
                venue = getattr(nexus.venues, name)
                self.assertEqual(venue.code, mic)
                self.assertEqual(nexus.parse_venue(mic), venue)
                entry = nexus.VENUES.select(venue)
                self.assertEqual(entry.venue, venue)
                self.assertEqual(str(venue), entry.display_name)

    def test_canadian_market_centers(self):
        for center, mic, name in (
                ('ALX', 'XATX', 'ALX'), ('ALD', 'ADRK', 'ALD'),
                ('ICX', 'XICX', 'ICX'), ('LIQ', 'LICA', 'LIQ'),
                ('AQN', 'NEON', 'NEON'), ('ASP', 'ASPC', 'ASPC'),
                ('ASV', 'ASPV', 'ASPV'), ('INC', 'INCC', 'INCC')):
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
