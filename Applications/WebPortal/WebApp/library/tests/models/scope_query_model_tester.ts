import * as Nexus from 'nexus';
import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { combineScopes, getScopeLabel, isScopeEqual, LocalTickerQueryModel,
  QueryModel, ScopeQueryModel, splitScope } from '../../source/models';

describe('ScopeQueryModel', () => {
  it('country_venue_and_ticker_aliases', async () => {
    const model = new ScopeQueryModel(new LocalTickerQueryModel([]));
    for(const query of ['CA', 'can', ' Canada ']) {
      assert.equal(getScopeLabel(await model.parse(query)), 'CA');
    }
    for(const query of ['tsx', 'XTSE']) {
      assert.equal(getScopeLabel(await model.parse(query)), 'TSX');
    }
    for(const query of ['ABX.TSX', ' abx.xtse ']) {
      assert.equal(getScopeLabel(await model.parse(query)), 'ABX.TSX');
    }
    for(const query of ['*', 'global', ' Global ']) {
      assert.ok((await model.parse(query)).isGlobal);
    }
    for(const query of ['', 'ABX', '.TSX', 'ABX.UNKNOWN', 'A B.TSX']) {
      assert.equal(await model.parse(query), null, query);
    }
  });

  it('local_prefixes_and_ticker_results', async () => {
    const queries: string[] = [];
    const ticker = Nexus.Ticker.parse('ZZZ.TSX');
    const tickers: QueryModel<Nexus.Ticker> = {
      parse: async query => { queries.push(query); return ticker; },
      submit: async query => { queries.push(query); return [ticker]; }
    };
    const model = new ScopeQueryModel(tickers);
    assert.deepEqual((await model.submit(' Canada ')).map(getScopeLabel),
      ['CA', 'ZZZ.TSX']);
    assert.equal(getScopeLabel(await model.parse(' id:42 ')), 'ZZZ.TSX');
    assert.deepEqual(queries, [' Canada ', ' id:42 ']);
    assert.deepEqual(await model.submit(' '), []);
    assert.deepEqual((await model.submit('*')).map(getScopeLabel), ['*']);
    assert.deepEqual(queries, [' Canada ', ' id:42 ']);
  });

  it('local_matches_survive_ticker_failure', async () => {
    const error = new Error('Lookup failed.');
    const tickers: QueryModel<Nexus.Ticker> = {
      parse: async () => { throw error; },
      submit: async () => { throw error; }
    };
    const model = new ScopeQueryModel(tickers);
    assert.deepEqual((await model.submit('Canada')).map(getScopeLabel), ['CA']);
    assert.equal(getScopeLabel(await model.parse('XTSE')), 'TSX');
    await assert.rejects(model.submit('ZZZ'), error);
    await assert.rejects(model.parse('ZZZ.TSX'), error);
  });
});

describe('Scope entries', () => {
  it('split_and_combine', () => {
    const scope = new Nexus.Scope('Named scope').
      add(new Nexus.Scope(Nexus.Ticker.parse('SHOP.TSX'))).
      add(new Nexus.Scope(Nexus.Ticker.parse('ABX.TSX'))).
      add(new Nexus.Scope(Nexus.Venues.ASX)).
      add(new Nexus.Scope(Nexus.Countries.CA));
    const entries = splitScope(scope);
    assert.deepEqual(entries.map(getScopeLabel),
      ['CA', 'ASX', 'ABX.TSX', 'SHOP.TSX']);
    const combined = combineScopes([...entries, ...entries], scope.name);
    assert.ok(isScopeEqual(scope, combined));
    assert.equal(combined.name, 'Named scope');
    assert.equal(getScopeLabel(combined), 'CA, ASX, ABX.TSX, SHOP.TSX');
    assert.deepEqual(splitScope(new Nexus.Scope()), []);
  });

  it('global', () => {
    const entries = [new Nexus.Scope(Nexus.Countries.CA), Nexus.Scope.GLOBAL,
      new Nexus.Scope(Nexus.Ticker.parse('ABX.TSX'))];
    const combined = combineScopes(entries, 'Everything');
    assert.ok(combined.isGlobal);
    assert.equal(combined.name, 'Everything');
    assert.equal(combined.countries.size, 0);
    assert.equal(combined.tickers.size, 0);
    assert.deepEqual(splitScope(combined).map(getScopeLabel), ['*']);
    assert.ok(isScopeEqual(combined, Nexus.Scope.GLOBAL));
    assert.equal(getScopeLabel(combineScopes([], '')), '');
  });

  it('membership_identity', () => {
    const first = new Nexus.Scope(Nexus.Ticker.parse('BMO.TSX'));
    const alias = new Nexus.Scope(Nexus.Ticker.parse('BMO.XTSE'));
    const otherVenue = new Nexus.Scope(Nexus.Ticker.parse('BMO.ASX'));
    assert.ok(isScopeEqual(first, alias));
    assert.ok(!isScopeEqual(first, otherVenue));
    assert.ok(!isScopeEqual(first, new Nexus.Scope(Nexus.Venues.TSX)));
    assert.ok(!isScopeEqual(first, new Nexus.Scope(Nexus.Countries.CA)));
    assert.ok(!isScopeEqual(new Nexus.Scope(), Nexus.Scope.GLOBAL));
  });
});
