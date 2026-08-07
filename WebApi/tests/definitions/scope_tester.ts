import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CountryCode } from '../../source/definitions/country';
import { Ticker } from '../../source/definitions/ticker';
import { Venue } from '../../source/definitions/venue';
import { Scope } from '../../source/definitions/scope';

describe('Scope', () => {
  it('empty', () => {
    const scope = new Scope();
    assert.strictEqual(scope.name, '');
    assert.strictEqual(scope.isGlobal, false);
    assert.strictEqual(scope.countries.size, 0);
    assert.strictEqual(scope.venues.size, 0);
    assert.strictEqual(scope.tickers.size, 0);
  });

  it('named', () => {
    const scope = new Scope('test');
    assert.strictEqual(scope.name, 'test');
    assert.strictEqual(scope.isGlobal, false);
  });

  it('global', () => {
    assert.strictEqual(Scope.GLOBAL.isGlobal, true);
  });

  it('make_global', () => {
    const scope = Scope.makeGlobal('all');
    assert.strictEqual(scope.isGlobal, true);
    assert.strictEqual(scope.name, 'all');
  });

  it('from_country', () => {
    const scope = new Scope(new CountryCode(840));
    assert.strictEqual(scope.countries.size, 1);
    assert.strictEqual(scope.venues.size, 0);
    assert.strictEqual(scope.tickers.size, 0);
  });

  it('from_venue', () => {
    const json = new Scope().toJson();
    json.venues = [new Venue('XNAS').toJson()];
    const scope = Scope.fromJson(json);
    assert.strictEqual(scope.venues.size, 1);
    assert.strictEqual(scope.countries.size, 0);
    assert.strictEqual(scope.tickers.size, 0);
  });

  it('from_ticker', () => {
    const scope = new Scope(new Ticker('AAPL', new Venue('XNAS')));
    assert.strictEqual(scope.tickers.size, 1);
    assert.strictEqual(scope.countries.size, 0);
    assert.strictEqual(scope.venues.size, 0);
  });

  it('add_scopes', () => {
    const a = new Scope(new CountryCode(840));
    const b = new Scope(new CountryCode(124));
    a.add(b);
    assert.strictEqual(a.countries.size, 2);
  });

  it('add_global_clears', () => {
    const scope = new Scope(new CountryCode(840));
    scope.add(Scope.GLOBAL);
    assert.strictEqual(scope.isGlobal, true);
    assert.strictEqual(scope.countries.size, 0);
    assert.strictEqual(scope.venues.size, 0);
    assert.strictEqual(scope.tickers.size, 0);
  });

  it('add_to_global_is_noop', () => {
    const scope = Scope.makeGlobal('');
    scope.add(new Scope(new CountryCode(840)));
    assert.strictEqual(scope.isGlobal, true);
    assert.strictEqual(scope.countries.size, 0);
  });

  it('clone', () => {
    const original = new Scope(new CountryCode(840));
    original.add(new Scope(new CountryCode(124)));
    const cloned = original.clone();
    assert.strictEqual(cloned.countries.size, 2);
    assert.strictEqual(cloned.name, original.name);
    assert.strictEqual(cloned.isGlobal, original.isGlobal);
  });

  it('to_string_named', () => {
    const scope = new Scope('test');
    assert.strictEqual(scope.toString(), 'test');
  });

  it('to_string_unnamed', () => {
    const scope = new Scope();
    assert.strictEqual(scope.toString(), '');
  });

  it('to_json', () => {
    const scope = new Scope('test');
    const json = scope.toJson();
    assert.strictEqual(json.name, 'test');
    assert.strictEqual(json.is_global, false);
  });

  it('from_json', () => {
    const json = {
      name: 'test',
      is_global: false,
      countries: [840],
      venues: ['XNAS'],
      tickers: [{ symbol: 'AAPL', venue: 'XNAS' }]
    };
    const restored = Scope.fromJson(json);
    assert.strictEqual(restored.countries.size, 1);
    assert.strictEqual(restored.venues.size, 1);
    assert.strictEqual(restored.tickers.size, 1);
    assert.strictEqual(restored.isGlobal, false);
    assert.strictEqual(restored.name, 'test');
  });

  it('from_json_global', () => {
    const json = Scope.GLOBAL.toJson();
    const restored = Scope.fromJson(json);
    assert.strictEqual(restored.isGlobal, true);
  });

  it('hash_global', () => {
    assert.strictEqual(Scope.GLOBAL.hash(), 0x9e3779b9);
  });

  it('hash_equal', () => {
    const a = new Scope(new CountryCode(840));
    const b = new Scope(new CountryCode(840));
    assert.strictEqual(a.hash(), b.hash());
  });

  it('countries_returns_clone', () => {
    const scope = new Scope(new CountryCode(840));
    const countries = scope.countries;
    countries.add(new CountryCode(124));
    assert.strictEqual(scope.countries.size, 1);
  });
});
