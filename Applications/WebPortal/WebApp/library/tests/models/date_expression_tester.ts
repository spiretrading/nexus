import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateExpression, DateOffset, resolveDateExpression } from
  '../../source/models/date_expression';
import { DayOfMonth } from '../../source/models/day_of_month';
import { Weekday } from '../../source/models/date_rule';

describe('DateExpression resolution', () => {
  it('resolves_ordinal_calendar_days_weekdays_and_named_weekdays', () => {
    const reference = new Beam.Date(2026, 10, 20);
    const {Position, DayType} = DayOfMonth;
    const cases: [DayOfMonth.Position,
      Weekday | DayOfMonth.DayType | number, number, string][] = [
      [Position.FIRST, DayType.DAY, 0, '20261001'],
      [Position.SECOND, DayType.DAY, 0, '20261002'],
      [Position.THIRD, DayType.DAY, 0, '20261003'],
      [Position.FOURTH, DayType.DAY, 0, '20261004'],
      [Position.LAST, DayType.DAY, 0, '20261031'],
      [Position.FIRST, DayType.WEEKDAY, 1, '20261102'],
      [Position.SECOND, DayType.WEEKDAY, 0, '20261002'],
      [Position.THIRD, DayType.WEEKDAY, 0, '20261005'],
      [Position.FOURTH, DayType.WEEKDAY, 0, '20261006'],
      [Position.LAST, DayType.WEEKDAY, 0, '20261030'],
      [Position.FIRST, Weekday.MONDAY, 0, '20261005'],
      [Position.SECOND, Weekday.TUESDAY, 0, '20261013'],
      [Position.THIRD, Weekday.WEDNESDAY, 0, '20261021'],
      [Position.FOURTH, Weekday.THURSDAY, 0, '20261022'],
      [Position.LAST, Weekday.FRIDAY, -1, '20260925'],
      [Position.DAY, 31, -8, '20260228']];
    for(const [position, day, offset, expected] of cases) {
      assert.equal(resolveDateExpression({
        mode: DateExpression.Mode.DAY_OF_MONTH,
        date: {day: new DayOfMonth(position, day),
          offset: {value: offset, unit: DateOffset.Unit.MONTH}}}, reference).
        toJson(), expected);
    }
  });

  it('resolves_fixed_day_and_week_offsets_and_rejects_incomplete_values',
      () => {
    const reference = new Beam.Date(2024, 3, 1);
    assert.equal(resolveDateExpression({mode: DateExpression.Mode.DAY_OFFSET,
      date: {value: -1, unit: DateOffset.Unit.DAY}}, reference).toJson(),
      '20240229');
    assert.equal(resolveDateExpression({mode: DateExpression.Mode.WEEKDAY,
      date: {day: Weekday.MONDAY,
        offset: {value: -1, unit: DateOffset.Unit.WEEK}}}, reference).toJson(),
      '20240219');
    assert.equal(resolveDateExpression({mode: DateExpression.Mode.FIXED,
      date: reference}, reference).toJson(), '20240301');
    for(const offset of [null, NaN, Infinity, 1.5]) {
      assert.equal(resolveDateExpression({mode: DateExpression.Mode.DAY_OFFSET,
        date: {value: offset, unit: DateOffset.Unit.DAY}}, reference), null);
    }
    assert.equal(resolveDateExpression({mode: DateExpression.Mode.DAY_OF_MONTH,
      date: {day: new DayOfMonth(DayOfMonth.Position.LAST,
        DayOfMonth.DayType.DAY),
        offset: {value: 1, unit: DateOffset.Unit.MONTH}}},
      new Beam.Date(9999, 12, 1)), null);
  });
});
