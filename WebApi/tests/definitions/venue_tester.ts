import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CountryCode } from '../../source/definitions/country';
import { Currency } from '../../source/definitions/currency';
import { Venue, VenueDatabase } from '../../source/definitions/venue';

describe('Venue', () => {
  it('construct', () => {
    const venue = new Venue('XNAS');
    assert.strictEqual(venue.toString(), 'XNAS');
  });

  it('none', () => {
    assert.strictEqual(Venue.NONE.toString(), '');
  });

  it('equals', () => {
    const a = new Venue('XNAS');
    const b = new Venue('XNAS');
    assert.ok(a.equals(b));
  });

  it('not_equal', () => {
    const a = new Venue('XNAS');
    const b = new Venue('XNYS');
    assert.ok(!a.equals(b));
  });

  it('to_json', () => {
    assert.strictEqual(new Venue('XNAS').toJson(), 'XNAS');
  });

  it('from_json', () => {
    const venue = Venue.fromJson('XNAS');
    assert.ok(venue.equals(new Venue('XNAS')));
  });

  it('round_trip_json', () => {
    const original = new Venue('XNYS');
    const restored = Venue.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });

  it('hash', () => {
    const a = new Venue('XNAS');
    const b = new Venue('XNAS');
    assert.strictEqual(a.hash(), b.hash());
  });
});

function makeEntry(code: string, displayName: string): VenueDatabase.Entry {
  return new VenueDatabase.Entry(
    new Venue(code), new CountryCode(840), '', 'UTC',
    new Currency(840), code, displayName);
}

describe('VenueDatabase', () => {
  it('none_entry', () => {
    const none = VenueDatabase.Entry.NONE;
    assert.ok(none.venue.equals(Venue.NONE));
    assert.strictEqual(none.displayName, '');
    assert.strictEqual(none.description, 'None');
  });

  it('venue_lookup', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    database.add(makeEntry('XNYS', 'NYSE'));
    assert.strictEqual(
      database.fromVenue(new Venue('XNAS')).displayName, 'NASDAQ');
    assert.strictEqual(
      database.fromVenue(new Venue('XNYS')).displayName, 'NYSE');
    assert.ok(
      database.fromVenue(new Venue('XZZZ')).equals(VenueDatabase.Entry.NONE));
  });

  it('display_name_lookup', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    assert.ok(
      database.fromDisplayName('NASDAQ').venue.equals(new Venue('XNAS')));
    assert.ok(
      database.fromDisplayName('UNKNOWN').equals(VenueDatabase.Entry.NONE));
  });

  it('remove', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    assert.ok(
      database.fromVenue(new Venue('XNAS')).venue.equals(new Venue('XNAS')));
    database.delete(new Venue('XNAS'));
    assert.ok(
      database.fromVenue(new Venue('XNAS')).equals(VenueDatabase.Entry.NONE));
  });

  it('add_duplicate_ignored', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    database.add(makeEntry('XNAS', 'Duplicate'));
    assert.strictEqual(
      database.fromVenue(new Venue('XNAS')).displayName, 'NASDAQ');
  });

  it('entry_equals', () => {
    const a = makeEntry('XNAS', 'NASDAQ');
    const b = makeEntry('XNAS', 'NASDAQ');
    assert.ok(a.equals(b));
  });

  it('entry_not_equal', () => {
    const a = makeEntry('XNAS', 'NASDAQ');
    const b = makeEntry('XNYS', 'NYSE');
    assert.ok(!a.equals(b));
  });

  it('entry_to_json', () => {
    const entry = makeEntry('XNAS', 'NASDAQ');
    const json = entry.toJson();
    assert.strictEqual(json.venue, 'XNAS');
    assert.strictEqual(json.display_name, 'NASDAQ');
    assert.strictEqual(json.country_code, 840);
    assert.strictEqual(json.currency, 840);
  });

  it('entry_from_json', () => {
    const json = {
      venue: 'XNAS',
      country_code: 840,
      market_center: '',
      time_zone: 'UTC',
      currency: 840,
      description: 'XNAS',
      display_name: 'NASDAQ'
    };
    const entry = VenueDatabase.Entry.fromJson(json);
    assert.ok(entry.venue.equals(new Venue('XNAS')));
    assert.strictEqual(entry.displayName, 'NASDAQ');
  });

  it('round_trip_json', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    database.add(makeEntry('XNYS', 'NYSE'));
    const json = { entries: database.toJson() };
    const restored = VenueDatabase.fromJson(json);
    assert.strictEqual(
      restored.fromVenue(new Venue('XNAS')).displayName, 'NASDAQ');
    assert.strictEqual(
      restored.fromVenue(new Venue('XNYS')).displayName, 'NYSE');
  });

  it('entry_parse_by_display_name', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    const entry = VenueDatabase.Entry.parse('NASDAQ', database);
    assert.ok(entry.venue.equals(new Venue('XNAS')));
  });

  it('entry_parse_by_venue_code', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    const entry = VenueDatabase.Entry.parse('XNAS', database);
    assert.ok(entry.venue.equals(new Venue('XNAS')));
  });

  it('entry_parse_not_found', () => {
    const database = new VenueDatabase();
    const entry = VenueDatabase.Entry.parse('UNKNOWN', database);
    assert.ok(entry.equals(VenueDatabase.Entry.NONE));
  });

  it('iterable', () => {
    const database = new VenueDatabase();
    database.add(makeEntry('XNAS', 'NASDAQ'));
    database.add(makeEntry('XNYS', 'NYSE'));
    const entries = [...database];
    assert.strictEqual(entries.length, 2);
  });
});
