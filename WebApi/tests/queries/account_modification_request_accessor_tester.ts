import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import * as Beam from 'beam';
import { AccountModificationRequestAccessor } from
  '../../source/queries/account_modification_request_accessor';
import { QueryType } from '../../source/queries/query_type';

/**
 * The members that Nexus' SqlTranslator maps to a column of the
 * account_modification_requests table. Accessing any other member throws when
 * the filter is translated.
 */
const MEMBERS = ['id', 'type', 'account', 'submission_account', 'timestamp',
  'effective_date', 'status', 'last_update_timestamp'];

describe('AccountModificationRequestAccessor', () => {
  it('from_parameter', () => {
    const accessor = AccountModificationRequestAccessor.fromParameter(1);
    const parameter = accessor.id.expression as Beam.ParameterExpression;
    assert.ok(parameter instanceof Beam.ParameterExpression);
    assert.strictEqual(parameter.index, 1);
    assert.strictEqual(
      parameter.type, QueryType.ACCOUNT_MODIFICATION_REQUEST);
  });

  it('accesses_only_translatable_members', () => {
    const accessor = AccountModificationRequestAccessor.fromParameter(0);
    const accessed = [accessor.id, accessor.type, accessor.account,
      accessor.submissionAccount, accessor.timestamp, accessor.effectiveDate,
      accessor.status, accessor.lastUpdateTimestamp];
    assert.deepStrictEqual(
      accessed.map((member) => member.name), MEMBERS);
  });

  it('member_types', () => {
    const accessor = AccountModificationRequestAccessor.fromParameter(0);
    assert.strictEqual(accessor.id.type, Beam.QueryType.INT);
    assert.strictEqual(accessor.type.type, Beam.QueryType.INT);
    assert.strictEqual(accessor.account.type, Beam.QueryType.INT);
    assert.strictEqual(accessor.submissionAccount.type, Beam.QueryType.INT);
    assert.strictEqual(accessor.timestamp.type, Beam.QueryType.DATE_TIME);
    assert.strictEqual(accessor.effectiveDate.type, Beam.QueryType.DATE_TIME);
    assert.strictEqual(accessor.status.type, Beam.QueryType.INT);
    assert.strictEqual(
      accessor.lastUpdateTimestamp.type, Beam.QueryType.DATE_TIME);
  });

  it('to_string', () => {
    const accessor = AccountModificationRequestAccessor.fromParameter(0);
    assert.strictEqual(accessor.account.toString(), '(parameter 0).account');
  });

  it('builds_a_filter_that_round_trips', () => {
    const accessor = AccountModificationRequestAccessor.fromParameter(0);
    const filter = new Beam.AndExpression(
      Beam.makeNotEquals(
        accessor.account, new Beam.ConstantExpression(new Beam.IntValue(12))),
      Beam.makeGreaterEquals(accessor.timestamp,
        new Beam.ConstantExpression(new Beam.DateTimeValue(
          Beam.DateTime.fromJson('20200102T030405')))));
    const query = new Beam.FilteredQuery(filter);
    const json = JSON.parse(JSON.stringify(query.toJson()));
    assert.ok(Beam.FilteredQuery.fromJson(json).filter.equals(filter));
    assert.strictEqual(filter.toString(),
      '(and (!= (parameter 0).account 12) ' +
      `(>= (parameter 0).timestamp ${new Beam.DateTimeValue(
        Beam.DateTime.fromJson('20200102T030405'))}))`);
  });
});
