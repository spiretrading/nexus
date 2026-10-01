import * as assert from 'node:assert/strict';
import { test } from 'node:test';
import * as Beam from 'beam';
import { makeParametersDateRangeOptions } from
  '../../source/pages/report_page/parameters_date_range_options';

test('reporting presets and calendar boundaries', () => {
  const cases = [
    {today: [2026, 9, 30], yesterday: [2026, 9, 29], week: [2026, 9, 28],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31]},
    {today: [2026, 1, 1], yesterday: [2025, 12, 31], week: [2025, 12, 29],
      previousStart: [2025, 12, 1], previousEnd: [2025, 12, 31]},
    {today: [2024, 3, 1], yesterday: [2024, 2, 29], week: [2024, 2, 26],
      previousStart: [2024, 2, 1], previousEnd: [2024, 2, 29]},
    {today: [2025, 3, 1], yesterday: [2025, 2, 28], week: [2025, 2, 24],
      previousStart: [2025, 2, 1], previousEnd: [2025, 2, 28]},
    {today: [2026, 9, 28], yesterday: [2026, 9, 27], week: [2026, 9, 28],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31]},
    {today: [2026, 9, 27], yesterday: [2026, 9, 26], week: [2026, 9, 21],
      previousStart: [2026, 8, 1], previousEnd: [2026, 8, 31]},
    {today: [99, 3, 1], yesterday: [99, 2, 28], week: [99, 2, 23],
      previousStart: [99, 2, 1], previousEnd: [99, 2, 28]}
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
      [entry.week, entry.today],
      [[year, month, 1], entry.today],
      [entry.previousStart, entry.previousEnd],
      [[year, 1, 1], entry.today]
    ]);
    assert.deepEqual(options.map(option => [option.value, option.label]), [
      ['today', 'Today'], ['yesterday', 'Yesterday'],
      ['week-to-date', 'Week to Date'], ['month-to-date', 'Month to Date'],
      ['previous-month', 'Previous Month'], ['year-to-date', 'Year to Date']
    ]);
  }
});
