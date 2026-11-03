import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateExpression, dateExpressionToRule, dateRuleToJson, DayOfMonth,
  DateOffset, Interval, OrdinalDayOfMonthDateRule, Weekday } from
  '../../source/models';
import { DayOfYear, Recurrence } from
  '../../source/pages/report_page/recurrence';
import { ReportRecurrence, fromReportRecurrence, toReportRecurrence,
  upcomingReportRuns } from
  '../../source/pages/report_page/report_recurrence';

function timestamp(date: string, time: string): Beam.DateTime {
  return new Beam.DateTime(Beam.Date.fromJson(date),
    Beam.Duration.fromJson(time));
}

function local(value: Beam.DateTime): Date {
  const {date, timeOfDay} = value;
  const time = timeOfDay.split();
  return new Date(date.year, date.month - 1, date.day,
    time.hours, time.minutes, time.seconds);
}

describe('ReportRecurrence', () => {
  it('preserves_ordinal_expressions_and_yearly_months', () => {
    const day = new DayOfMonth(DayOfMonth.Position.LAST,
      DayOfMonth.DayType.WEEKDAY);
    const recurrence = new Recurrence(new Interval(1, Interval.Unit.YEAR),
      new DayOfYear(day, Beam.Date.Month.FEBRUARY));
    const wire = toReportRecurrence(recurrence);
    assert.equal(wire.month, 2);
    assert.deepEqual(dateRuleToJson(wire.rule), {
      type: 'ORDINAL_DAY_OF_MONTH',
      value: {offset: 0, position: {type: 'LAST', ordinal: 1}, day: 'WEEKDAY'}});
    assert.deepEqual(fromReportRecurrence(wire, new Beam.Date(2026, 10, 20)),
      recurrence);
    const expression: DateExpression = {mode: DateExpression.Mode.DAY_OF_MONTH,
      date: {day, offset: {value: -1, unit: DateOffset.Unit.MONTH}}};
    assert.deepEqual(dateRuleToJson(dateExpressionToRule(expression)), {
      type: 'ORDINAL_DAY_OF_MONTH',
      value: {offset: -1, position: {type: 'LAST', ordinal: 1}, day: 'WEEKDAY'}});
  });

  it('skips_nonexistent_times_and_uses_the_first_fold', () => {
    const previous = process.env.TZ;
    process.env.TZ = 'America/Toronto';
    try {
      const daily: ReportRecurrence = {
        interval: new Interval(1, Interval.Unit.DAY), rule: null, month: null};
      const start = timestamp('20260307', '02:30:00');
      assert.deepEqual(upcomingReportRuns(start, daily, local(start), 3).
        map(value => value.date.toJson()),
        ['20260309', '20260310', '20260311']);
      const fold = timestamp('20261031', '01:45:00');
      assert.deepEqual(upcomingReportRuns(fold, daily,
        new Date('2026-11-01T06:30:00Z'), 2).
        map(value => value.date.toJson()), ['20261102', '20261103']);
      const fall = timestamp('20261031', '01:30:00');
      assert.deepEqual(upcomingReportRuns(fall, daily, local(fall), 2).
        map(value => value.date.toJson()), ['20261101', '20261102']);
    } finally {
      if(previous == null) {
        delete process.env.TZ;
      } else {
        process.env.TZ = previous;
      }
    }
  });

  it('previews_calendar_occurrences_after_start_and_now', () => {
    const start = timestamp('20261020', '09:00:00');
    const rule = new OrdinalDayOfMonthDateRule(0,
      OrdinalDayOfMonthDateRule.Position.LAST,
      OrdinalDayOfMonthDateRule.Day.WEEKDAY);
    const yearly = {interval: new Interval(1, Interval.Unit.YEAR), rule,
      month: 2};
    assert.deepEqual(upcomingReportRuns(start, yearly, local(start), 3).
      map(value => value.date.toJson()), ['20270226', '20280229', '20290228']);
    assert.deepEqual(upcomingReportRuns(start,
      {...yearly, interval: new Interval(1, Interval.Unit.MONTH), month: null},
      local(start), 3).map(value => value.date.toJson()),
      ['20261030', '20261130', '20261231']);
    const weekly = toReportRecurrence(new Recurrence(
      new Interval(2, Interval.Unit.WEEK), Weekday.MONDAY));
    assert.deepEqual(upcomingReportRuns(start, weekly, local(start), 3).
      map(value => value.date.toJson()), ['20261102', '20261116', '20261130']);
    const daily: ReportRecurrence = {
      interval: new Interval(1, Interval.Unit.DAY), rule: null,
      month: null};
    assert.deepEqual(upcomingReportRuns(start, daily,
      local(timestamp('20261019', '09:00:00')), 3).
      map(value => value.date.toJson()),
      ['20261020', '20261021', '20261022']);
  });
});
