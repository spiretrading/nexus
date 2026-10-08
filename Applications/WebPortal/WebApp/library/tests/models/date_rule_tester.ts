import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { applyDateRule, DateRule, DateRuleType, dateRuleToJson,
  DayOffsetDateRule, DayOfMonthDateRule, isDateRuleEqual,
  MonthBoundaryDateRule, parseDateRule, resolveDateRule, SpecificDateRule,
  Weekday, WeekdayDateRule } from '../../source/models/date_rule';

const REFERENCE = new Beam.Date(2026, 10, 7);
const BOUNDARY = MonthBoundaryDateRule.Boundary;

describe('DateRule', () => {
  it('applies_the_same_calendar_rules_as_cpp_and_preserves_time', () => {
    const reference = new Beam.DateTime(REFERENCE,
      Beam.Duration.fromJson('12:34:56.123'));
    const cases: [DateRule, string][] = [
      [new SpecificDateRule(new Beam.Date(2024, 2, 29)), '20240229'],
      [new DayOffsetDateRule(-1), '20261006'],
      [new DayOffsetDateRule(0), '20261007'],
      [new DayOffsetDateRule(2), '20261009'],
      [new WeekdayDateRule(-2, Weekday.MONDAY), '20260921'],
      [new WeekdayDateRule(-1, Weekday.TUESDAY), '20260929'],
      [new WeekdayDateRule(0, Weekday.THURSDAY), '20261008'],
      [new WeekdayDateRule(0, Weekday.SUNDAY), '20261011'],
      [new WeekdayDateRule(1, Weekday.MONDAY), '20261012'],
      [new DayOfMonthDateRule(-1, 15), '20260915'],
      [new DayOfMonthDateRule(-1, 31), '20260930'],
      [new DayOfMonthDateRule(3, 1), '20270101'],
      [new MonthBoundaryDateRule(0, BOUNDARY.FIRST, 0), '20261001'],
      [new MonthBoundaryDateRule(0, BOUNDARY.LAST, -1), '20261030'],
      [new MonthBoundaryDateRule(0, BOUNDARY.FIRST, -1), '20260930'],
      [new MonthBoundaryDateRule(0, BOUNDARY.LAST, 1), '20261101'],
      [new MonthBoundaryDateRule(-10, BOUNDARY.FIRST, -1), '20251130']];
    for(const [rule, expected] of cases) {
      const result = applyDateRule(rule, reference);
      assert.equal(result.date.toJson(), expected);
      assert.equal(result.timeOfDay.ticks, reference.timeOfDay.ticks);
    }
    assert.equal(resolveDateRule(new WeekdayDateRule(0, Weekday.MONDAY),
      new Beam.Date(2026, 10, 11)).toJson(), '20261005');
    for(const [year, end] of [
        [2024, '20240229'], [2025, '20250228']] as const) {
      assert.equal(resolveDateRule(new DayOfMonthDateRule(-1, 31),
        new Beam.Date(year, 3, 31)).toJson(), end);
      assert.equal(resolveDateRule(new MonthBoundaryDateRule(-1,
        BOUNDARY.LAST, 0), new Beam.Date(year, 3, 31)).toJson(), end);
    }
  });

  it('round_trips_the_cpp_wire_format_with_names', () => {
    const cases = [
      {type: 'SPECIFIC_DATE', value: {date: '20240229'}},
      {type: 'DAY_OFFSET', value: {offset: -2147483648}},
      {type: 'WEEKDAY', value: {offset: -2, day: 'SUNDAY'}},
      {type: 'DAY_OF_MONTH', value: {offset: 1, day: 31}},
      {type: 'MONTH_BOUNDARY', value: {offset: -1, boundary: 'LAST',
        day_offset: -1}},
      {type: 'DAY_OFFSET', value: {offset: 2147483647}}];
    for(const json of cases) {
      const rule = parseDateRule(json);
      assert.equal(rule.type, json.type);
      assert.deepEqual(dateRuleToJson(rule), json);
      assert.ok(isDateRuleEqual(rule, parseDateRule(dateRuleToJson(rule))));
      assert.deepEqual(dateRuleToJson(parseDateRule({...json, __version: 0,
        value: {...json.value, __version: 0}})), json);
    }
    assert.equal(dateRuleToJson(parseDateRule(null)), null);
    for(const invalid of [
        {type: 1, value: {offset: 0}}, {type: 'Unknown', value: {}},
        {type: 'DAY_OFFSET', value: {offset: 0.5}},
        {type: 'DAY_OFFSET', value: {offset: 2147483648}},
        {type: 'DAY_OFFSET', value: {offset: -2147483649}},
        {type: 'DAY_OFFSET', value: {offset: null}},
        {type: 'WEEKDAY', value: {offset: 0, day: 0}},
        {type: 'DAY_OF_MONTH', value: {offset: 0, day: 32}},
        {type: 'MONTH_BOUNDARY', value: {offset: 0, boundary: 1, day_offset: 0}},
        {type: 'SPECIFIC_DATE', value: {date: '20260230'}}]) {
      assert.throws(() => parseDateRule(invalid));
    }
  });

  it('rejects_incomplete_rules_and_results_outside_cpp_date_limits', () => {
    const reference = new Beam.DateTime(REFERENCE, Beam.Duration.ZERO);
    for(const rule of [new SpecificDateRule(null),
        new SpecificDateRule(new Beam.Date(2026, 2, 30)),
        new DayOffsetDateRule(null), new DayOffsetDateRule(0.5),
        new DayOffsetDateRule(Infinity), new DayOfMonthDateRule(0, 0),
        new MonthBoundaryDateRule(0, BOUNDARY.LAST, null)]) {
      assert.throws(() => applyDateRule(rule, reference));
      assert.equal(resolveDateRule(rule, REFERENCE), null);
    }
    const minimum = new Beam.Date(1400, 1, 1);
    const maximum = new Beam.Date(9999, 12, 31);
    assert.equal(resolveDateRule(new DayOffsetDateRule(0), minimum).toJson(),
      '14000101');
    assert.equal(resolveDateRule(new DayOffsetDateRule(-1), minimum), null);
    assert.equal(resolveDateRule(new DayOffsetDateRule(1), maximum), null);
    assert.equal(resolveDateRule(new MonthBoundaryDateRule(1,
      BOUNDARY.FIRST, -31), maximum), null);
    for(const offset of [-2147483648, 2147483647]) {
      for(const rule of [new DayOffsetDateRule(offset),
          new WeekdayDateRule(offset, Weekday.MONDAY),
          new DayOfMonthDateRule(offset, 1),
          new MonthBoundaryDateRule(offset, BOUNDARY.LAST, 0)]) {
        assert.throws(() => applyDateRule(rule, reference), RangeError);
      }
    }
  });

  it('compares_only_the_active_rule_parameters', () => {
    const day = new DayOffsetDateRule(-1);
    assert.ok(isDateRuleEqual(day, new DayOffsetDateRule(-1)));
    assert.ok(!isDateRuleEqual(day, new DayOffsetDateRule(1)));
    assert.ok(!isDateRuleEqual(day, new WeekdayDateRule(-1, Weekday.MONDAY)));
    const boundary = new MonthBoundaryDateRule(0, BOUNDARY.LAST, -1);
    assert.ok(!isDateRuleEqual(boundary, {...boundary, dayOffset: 1}));
    assert.equal(boundary.type, DateRuleType.MONTH_BOUNDARY);
  });
});
