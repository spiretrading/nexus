import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CountryCode, CountryDatabase } from '../../source/definitions/country';

describe('CountryCode', () => {
  it('construct', () => {
    const code = new CountryCode(840);
    assert.strictEqual(code.code, 840);
  });

  it('none', () => {
    assert.strictEqual(CountryCode.NONE.code, 65535);
  });

  it('equals', () => {
    const a = new CountryCode(42);
    const b = new CountryCode(42);
    assert.ok(a.equals(b));
  });

  it('not_equal', () => {
    const a = new CountryCode(42);
    const b = new CountryCode(7);
    assert.ok(!a.equals(b));
  });

  it('to_json', () => {
    const code = new CountryCode(840);
    assert.strictEqual(code.toJson(), 840);
  });

  it('from_json', () => {
    const code = CountryCode.fromJson(840);
    assert.strictEqual(code.code, 840);
  });

  it('round_trip_json', () => {
    const original = new CountryCode(124);
    const restored = CountryCode.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });

  it('hash', () => {
    const a = new CountryCode(840);
    const b = new CountryCode(840);
    assert.strictEqual(a.hash(), b.hash());
  });
});

describe('CountryDatabase', () => {
  it('none_entry', () => {
    const none = CountryDatabase.Entry.NONE;
    assert.ok(none.code.equals(CountryCode.NONE));
    assert.strictEqual(none.name, 'None');
    assert.strictEqual(none.twoLetterCode, '??');
    assert.strictEqual(none.threeLetterCode, '???');
  });

  it('code_lookup', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(2), 'Two', 'TT', 'TTT'));
    database.add(new CountryDatabase.Entry(
      new CountryCode(1), 'One', 'OO', 'OOO'));
    assert.strictEqual(database.fromCode(new CountryCode(1)).name, 'One');
    assert.strictEqual(database.fromCode(new CountryCode(2)).name, 'Two');
    assert.ok(
      database.fromCode(new CountryCode(3)).code.equals(CountryCode.NONE));
  });

  it('letter_code_lookup', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(5), 'FiveLand', 'FL', 'FIV'));
    assert.ok(
      database.fromLetterCode('FL').code.equals(new CountryCode(5)));
    assert.ok(
      database.fromLetterCode('FIV').code.equals(new CountryCode(5)));
    assert.ok(
      database.fromLetterCode('XX').code.equals(CountryCode.NONE));
  });

  it('remove', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(9), 'Nine', 'NI', 'NIN'));
    assert.ok(
      database.fromCode(new CountryCode(9)).code.equals(new CountryCode(9)));
    database.delete(new CountryCode(9));
    assert.ok(
      database.fromCode(new CountryCode(9)).code.equals(CountryCode.NONE));
  });

  it('add_duplicate_ignored', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(1), 'One', 'OO', 'OOO'));
    database.add(new CountryDatabase.Entry(
      new CountryCode(1), 'Duplicate', 'DD', 'DDD'));
    assert.strictEqual(database.fromCode(new CountryCode(1)).name, 'One');
  });

  it('entry_equals', () => {
    const a = new CountryDatabase.Entry(
      new CountryCode(840), 'United States', 'US', 'USA');
    const b = new CountryDatabase.Entry(
      new CountryCode(840), 'United States', 'US', 'USA');
    assert.ok(a.equals(b));
  });

  it('entry_not_equal', () => {
    const a = new CountryDatabase.Entry(
      new CountryCode(840), 'United States', 'US', 'USA');
    const b = new CountryDatabase.Entry(
      new CountryCode(124), 'Canada', 'CA', 'CAN');
    assert.ok(!a.equals(b));
  });

  it('entry_to_json', () => {
    const entry = new CountryDatabase.Entry(
      new CountryCode(840), 'United States', 'US', 'USA');
    const json = entry.toJson();
    assert.strictEqual(json.code, 840);
    assert.strictEqual(json.name, 'United States');
    assert.strictEqual(json.two_letter_code, 'US');
    assert.strictEqual(json.three_letter_code, 'USA');
  });

  it('entry_from_json', () => {
    const json = {
      code: 840,
      name: 'United States',
      two_letter_code: 'US',
      three_letter_code: 'USA'
    };
    const entry = CountryDatabase.Entry.fromJson(json);
    assert.ok(entry.code.equals(new CountryCode(840)));
    assert.strictEqual(entry.name, 'United States');
    assert.strictEqual(entry.twoLetterCode, 'US');
    assert.strictEqual(entry.threeLetterCode, 'USA');
  });

  it('round_trip_json', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(840), 'United States', 'US', 'USA'));
    database.add(new CountryDatabase.Entry(
      new CountryCode(124), 'Canada', 'CA', 'CAN'));
    const json = { entries: database.toJson() };
    const restored = CountryDatabase.fromJson(json);
    assert.ok(
      restored.fromCode(new CountryCode(840)).name === 'United States');
    assert.ok(restored.fromCode(new CountryCode(124)).name === 'Canada');
  });

  it('iterable', () => {
    const database = new CountryDatabase();
    database.add(new CountryDatabase.Entry(
      new CountryCode(1), 'One', 'OO', 'OOO'));
    database.add(new CountryDatabase.Entry(
      new CountryCode(2), 'Two', 'TT', 'TTT'));
    const entries = [...database];
    assert.strictEqual(entries.length, 2);
  });
});
