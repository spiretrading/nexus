import * as Beam from 'beam';
import { DateRangeOption } from '../../models/date_range';

/** Builds the reporting presets relative to a local calendar date.
 * @param today - The reference date for the presets.
 * @return The reporting presets in display order, excluding Custom.
 */
export function makeParametersDateRangeOptions(today: Beam.Date):
    DateRangeOption[] {
  const date = new Date(0);
  date.setUTCFullYear(today.year, today.month - 1, today.day);
  const weekday = date.getUTCDay();
  const day = (month: number, value: number) => {
    const result = new Date(0);
    result.setUTCFullYear(today.year, month - 1, value);
    return new Beam.Date(result.getUTCFullYear(), result.getUTCMonth() + 1,
      result.getUTCDate());
  };
  const yesterday = day(today.month, today.day - 1);
  return [
    new DateRangeOption('today', 'Today', today, today),
    new DateRangeOption('yesterday', 'Yesterday', yesterday, yesterday),
    new DateRangeOption('week-to-date', 'Week to Date',
      day(today.month, today.day - (weekday + 6) % 7), today),
    new DateRangeOption('month-to-date', 'Month to Date',
      new Beam.Date(today.year, today.month, 1), today),
    new DateRangeOption('previous-month', 'Previous Month',
      day(today.month - 1, 1), day(today.month, 0)),
    new DateRangeOption('year-to-date', 'Year to Date',
      new Beam.Date(today.year, 1, 1), today)];
}
