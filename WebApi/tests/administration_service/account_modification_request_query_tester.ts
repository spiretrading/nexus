import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import * as Beam from 'beam';
import { AccountModificationRequestAnchor,
  AccountModificationRequestQuery } from
  '../../source/administration_service/account_modification_request_query';

const ACCOUNT = Beam.DirectoryEntry.makeAccount(100, 'trader');
const LIMIT = Beam.SnapshotLimit.fromHead(20);

function makeQuery(): AccountModificationRequestQuery {
  return new AccountModificationRequestQuery(ACCOUNT, LIMIT);
}

describe('AccountModificationRequestAnchor', () => {
  it('default_constructor', () => {
    const anchor = new AccountModificationRequestAnchor();
    assert.strictEqual(anchor.id, -1);
    assert.ok(anchor.date.equals(Beam.DateTime.NEG_INFIN));
    assert.strictEqual(anchor.name, '');
  });

  it('equals', () => {
    const date = Beam.DateTime.fromJson('20260102T030405');
    const anchor = new AccountModificationRequestAnchor(1, date, 'trader');
    assert.ok(anchor.equals(
      new AccountModificationRequestAnchor(1, date, 'trader')));
    assert.ok(!anchor.equals(
      new AccountModificationRequestAnchor(2, date, 'trader')));
    assert.ok(!anchor.equals(new AccountModificationRequestAnchor(
      1, Beam.DateTime.fromJson('20260102T030406'), 'trader')));
    assert.ok(!anchor.equals(
      new AccountModificationRequestAnchor(1, date, 'manager')));
    assert.ok(!anchor.equals(null));
  });

  it('round_trip', () => {
    const anchor = new AccountModificationRequestAnchor(
      1, Beam.DateTime.fromJson('20260102T030405'), 'trader');
    const json = JSON.parse(JSON.stringify(anchor.toJson()));
    assert.ok(AccountModificationRequestAnchor.fromJson(json).equals(anchor));
  });
});

describe('AccountModificationRequestQuery', () => {
  it('equals', () => {
    assert.ok(makeQuery().equals(makeQuery()));
    assert.ok(!makeQuery().equals(null));
  });

  it('equals_compares_its_own_fields', () => {
    const search = makeQuery();
    search.search = 'trader';
    assert.ok(!makeQuery().equals(search));
    const sorted = makeQuery();
    sorted.sortField = AccountModificationRequestQuery.SortField.LAST_UPDATED;
    assert.ok(!makeQuery().equals(sorted));
  });

  it('sends_only_the_fields_the_service_reads', () => {
    const json = makeQuery().toJson();
    assert.deepStrictEqual(Object.keys(json).filter(key =>
      !['index', 'snapshot_limit', 'filter', 'anchor', 'offset'].
        includes(key)).sort(), ['search', 'sort_field']);
  });

  it('equals_compares_the_inherited_fields', () => {
    const offset = makeQuery();
    offset.offset = 20;
    assert.ok(!makeQuery().equals(offset));
    const filtered = makeQuery();
    filtered.filter = Beam.ConstantExpression.FALSE;
    assert.ok(!makeQuery().equals(filtered));
    const anchored = makeQuery();
    anchored.anchor = new AccountModificationRequestAnchor(
      1, Beam.DateTime.fromJson('20260102T030405'), 'trader');
    assert.ok(!makeQuery().equals(anchored));
    const other = makeQuery();
    other.anchor = new AccountModificationRequestAnchor(
      1, Beam.DateTime.fromJson('20260102T030405'), 'trader');
    assert.ok(anchored.equals(other));
  });

  it('to_string', () => {
    const query = makeQuery();
    assert.strictEqual(
      query.toString(), `((${ACCOUNT} (HEAD 20) true) CREATED)`);
    query.sortField = AccountModificationRequestQuery.SortField.EFFECTIVE_DATE;
    assert.strictEqual(
      query.toString(), `((${ACCOUNT} (HEAD 20) true) EFFECTIVE_DATE)`);
  });
});
