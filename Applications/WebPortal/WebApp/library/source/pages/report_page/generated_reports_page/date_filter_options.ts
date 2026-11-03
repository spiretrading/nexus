import * as Beam from 'beam';
import { DateRangeOption } from '../../../models/date_range';

/** Builds the calendar-month presets for filtering generated reports.
 * @param today - The local calendar date used to determine the current month.
 * @return The filter presets in display order, excluding Custom.
 */
export function makeDateFilterOptions(today: Beam.Date): DateRangeOption[] {
  const month = (offset: number) => {
    const start = new Date(0);
    start.setUTCFullYear(today.year, today.month - 1 - offset, 1);
    const end = new Date(0);
    end.setUTCFullYear(today.year, today.month - offset, 0);
    return new DateRangeOption(
      `${String(start.getUTCFullYear()).padStart(4, '0')}-` +
        String(start.getUTCMonth() + 1).padStart(2, '0'),
      start.toLocaleString('en-US', {
        month: 'long', year: 'numeric', timeZone: 'UTC'}),
      new Beam.Date(start.getUTCFullYear(), start.getUTCMonth() + 1, 1),
      new Beam.Date(end.getUTCFullYear(), end.getUTCMonth() + 1,
        end.getUTCDate()));
  };
  const current = month(0);
  const previous = month(1);
  return [
    new DateRangeOption('all-time', 'All Time', null, null),
    new DateRangeOption('this-month', 'This Month',
      current.startDate, current.endDate),
    new DateRangeOption('previous-month', 'Previous Month',
      previous.startDate, previous.endDate),
    ...Array.from({length: 5}, (_, index) => month(index + 2))];
}
