import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateRule, isDateRuleEqual, resolveDateRule } from
  '../../source/models/date_rule';

const REFERENCE = new Beam.Date(2026, 10, 7);

describe('DateRule', () => {
  it('resolves_fixed_dates_and_day_offsets', () => {
    const date = new Beam.Date(2026, 9, 15);
    const fixed = new DateRule(DateRule.Type.SPECIFIC_DATE, date);
    assert.ok(resolveDateRule(fixed, REFERENCE).equals(date));
    assert.equal(resolveDateRule({...fixed, date: null}, REFERENCE), null);
    const rule = new DateRule(DateRule.Type.DAY_OFFSET, null);
    assert.equal(resolveDateRule(rule, REFERENCE).toJson(), '20261007');
    assert.equal(resolveDateRule({...rule, count: 7}, REFERENCE).toJson(),
      '20260930');
    assert.equal(resolveDateRule({...rule, count: 7,
      direction: DateRule.Direction.AFTER}, REFERENCE).toJson(), '20261014');
  });

  it('resolves_weekdays_from_monday', () => {
    const rule = new DateRule(DateRule.Type.WEEKDAY, null);
    assert.equal(resolveDateRule({...rule, count: 2}, REFERENCE).toJson(),
      '20260921');
    assert.equal(resolveDateRule({...rule, weekday: 4}, REFERENCE).toJson(),
      '20261009');
    assert.equal(resolveDateRule({...rule, count: 1, weekday: 1},
      REFERENCE).toJson(), '20260929');
    assert.equal(resolveDateRule({...rule, weekday: 6},
      new Beam.Date(2026, 10, 11)).toJson(), '20261011');
    assert.equal(resolveDateRule({...rule, count: 1,
      direction: DateRule.Direction.AFTER}, REFERENCE).toJson(), '20261012');
  });

  it('resolves_month_days_and_boundaries', () => {
    const rule = new DateRule(DateRule.Type.DAY_OF_MONTH, null);
    assert.equal(resolveDateRule({...rule, count: 1, day: 15},
      REFERENCE).toJson(), '20260915');
    for(const [reference, expected] of [
        [new Beam.Date(2024, 3, 31), '20240229'],
        [new Beam.Date(2025, 3, 31), '20250228'],
        [new Beam.Date(2026, 1, 31), '20251231']] as const) {
      assert.equal(resolveDateRule({...rule, count: 1, day: 31},
        reference).toJson(), expected);
    }
    const boundary = new DateRule(DateRule.Type.MONTH_BOUNDARY, null);
    assert.equal(resolveDateRule(boundary, REFERENCE).toJson(), '20261031');
    assert.equal(resolveDateRule({...boundary, count: 1,
      boundary: DateRule.Boundary.FIRST}, REFERENCE).toJson(), '20260901');
  });

  it('offsets_before_and_after_month_boundaries', () => {
    const rule = new DateRule(DateRule.Type.MONTH_BOUNDARY, null);
    assert.equal(resolveDateRule({...rule, boundaryOffset: 1},
      REFERENCE).toJson(), '20261030');
    for(const [reference, expected] of [
        [new Beam.Date(2024, 2, 15), '20240228'],
        [new Beam.Date(2025, 2, 15), '20250227']] as const) {
      assert.equal(resolveDateRule({...rule, boundaryOffset: 1},
        reference).toJson(), expected);
    }
    assert.equal(resolveDateRule({...rule, boundaryOffset: 2,
      boundaryDirection: DateRule.Direction.AFTER,
      boundary: DateRule.Boundary.FIRST}, REFERENCE).toJson(), '20261003');
    assert.equal(resolveDateRule({...rule, count: 1, boundaryOffset: 1},
      REFERENCE).toJson(), '20260929');
    assert.equal(resolveDateRule({...rule, count: 1,
      direction: DateRule.Direction.AFTER, boundaryOffset: 1,
      boundaryDirection: DateRule.Direction.AFTER}, REFERENCE).toJson(),
      '20261201');
    assert.equal(resolveDateRule({...rule, boundary: DateRule.Boundary.FIRST,
      boundaryOffset: 1}, REFERENCE).toJson(), '20260930');
    for(const boundaryOffset of [null, -1, 1.5, Infinity]) {
      assert.equal(resolveDateRule({...rule, boundaryOffset}, REFERENCE), null);
    }
    assert.ok(!isDateRuleEqual(rule, {...rule, boundaryOffset: 1}));
    assert.ok(!isDateRuleEqual(rule, {...rule,
      boundaryDirection: DateRule.Direction.AFTER}));
  });

  it('rejects_incomplete_and_out_of_range_rules', () => {
    const rule = new DateRule(DateRule.Type.DAY_OFFSET, null);
    for(const count of [null, -1, 1.5, Infinity, Number.MAX_SAFE_INTEGER]) {
      assert.equal(resolveDateRule({...rule, count}, REFERENCE), null);
    }
    assert.equal(resolveDateRule({...rule, count: 1},
      new Beam.Date(0, 1, 1)), null);
    assert.equal(resolveDateRule({...rule, count: 1,
      direction: DateRule.Direction.AFTER}, new Beam.Date(9999, 12, 31)), null);
    assert.equal(resolveDateRule({...rule, type: DateRule.Type.DAY_OF_MONTH,
      day: null}, REFERENCE), null);
    assert.equal(resolveDateRule({...rule, type: DateRule.Type.WEEKDAY,
      weekday: 7}, REFERENCE), null);
  });

  it('compares_rules_independently_of_resolved_dates', () => {
    const rule = new DateRule(DateRule.Type.DAY_OFFSET, null);
    assert.ok(isDateRuleEqual(rule, {...rule}));
    assert.ok(!isDateRuleEqual(rule, {...rule,
      direction: DateRule.Direction.AFTER}));
    assert.ok(!isDateRuleEqual(rule, {...rule, count: 1}));
  });
});
