import * as Beam from 'beam';

/** Formats a generated report's creation date for display and filtering. */
export function formatReportDate(date: Beam.Date): string {
  const value = new Date(0);
  value.setUTCFullYear(date.year, date.month - 1, date.day);
  return value.toLocaleDateString('en-US', {
    month: 'short', day: '2-digit', year: 'numeric', timeZone: 'UTC'});
}
