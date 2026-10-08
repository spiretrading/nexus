import * as assert from 'node:assert/strict';
import { test } from 'node:test';
import * as Beam from 'beam';
import { DateRule, resolveDateRule } from '../../source/models/date_rule';
import { makeParametersDateRangeOptions } from
  '../../source/pages/report_page/parameters_date_range_options';

test('reporting presets and calendar boundaries', () => {
  const cases = [
    {today: [2026, 9, 30], yesterday: [2026, 9, 29],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31],
      monthEnd: [2026, 9, 30]},
    {today: [2026, 1, 1], yesterday: [2025, 12, 31],
      previousStart: [2025, 12, 1], previousEnd: [2025, 12, 31],
      monthEnd: [2026, 1, 31]},
    {today: [2024, 3, 1], yesterday: [2024, 2, 29],
      previousStart: [2024, 2, 1], previousEnd: [2024, 2, 29],
      monthEnd: [2024, 3, 31]},
    {today: [2025, 3, 1], yesterday: [2025, 2, 28],
      previousStart: [2025, 2, 1], previousEnd: [2025, 2, 28],
      monthEnd: [2025, 3, 31]},
    {today: [2026, 9, 28], yesterday: [2026, 9, 27],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31],
      monthEnd: [2026, 9, 30]},
    {today: [2026, 9, 27], yesterday: [2026, 9, 26],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31],
      monthEnd: [2026, 9, 30]},
    {today: [99, 3, 1], yesterday: [99, 2, 28],
      previousStart: [99, 2, 1], previousEnd: [99, 2, 28],
      monthEnd: [99, 3, 31]}
  ];
  for(const entry of cases) {
    const [year, month, day] = entry.today;
    const today = new Beam.Date(year, month, day);
    const options = makeParametersDateRangeOptions(today);
    const dates = options.map(option => [option.startDate, option.endDate].
      map(date => [date.year, date.month, date.day]));
    assert.deepEqual(dates, [
      [entry.today, entry.today],
      [entry.yesterday, entry.yesterday],
      [[year, month, 1], entry.today],
      [[year, month, 1], entry.monthEnd],
      [entry.previousStart, entry.previousEnd]
    ]);
    assert.deepEqual(options.map(option => [option.value, option.label]), [
      ['today', 'Today'], ['yesterday', 'Yesterday'],
      ['month-to-date', 'Month to Date'],
      ['this-month', 'This Month'], ['previous-month', 'Last Month']
    ]);
  }
});

test('relative presets retain their meaning for another reference date', () => {
  const options = makeParametersDateRangeOptions(new Beam.Date(2026, 10, 7));
  const reference = new Beam.Date(2026, 11, 5);
  for(const [id, start, end] of [
      ['today', '20261105', '20261105'],
      ['yesterday', '20261104', '20261104'],
      ['month-to-date', '20261101', '20261105'],
      ['this-month', '20261101', '20261130'],
      ['previous-month', '20261001', '20261031']]) {
    const rules = options.find(option => option.value === id).rules;
    assert.ok(rules);
    assert.notEqual(rules.start.type, DateRule.Type.SPECIFIC_DATE);
    assert.notEqual(rules.end.type, DateRule.Type.SPECIFIC_DATE);
    assert.equal(resolveDateRule(rules.start, reference).toJson(), start);
    assert.equal(resolveDateRule(rules.end, reference).toJson(), end);
  }
});
