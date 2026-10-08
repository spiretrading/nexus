import * as Beam from 'beam';
import { DateRangeOption, DateRangeRules } from '../../models/date_range';
import { DateRule, resolveDateRule } from '../../models/date_rule';

/** Builds the reporting presets relative to a local calendar date.
 * @param today - The reference date for the presets.
 * @return The reporting presets in display order, excluding Custom.
 */
export function makeParametersDateRangeOptions(today: Beam.Date):
    DateRangeOption[] {
  const currentDay = new DateRule(DateRule.Type.DAY_OFFSET, null);
  const yesterday = {...currentDay, count: 1};
  const firstDay = {...new DateRule(DateRule.Type.MONTH_BOUNDARY, null),
    boundary: DateRule.Boundary.FIRST};
  const lastDay = new DateRule(DateRule.Type.MONTH_BOUNDARY, null);
  const option = (value: string, label: string, start: DateRule,
      end: DateRule) => new DateRangeOption(value, label,
    resolveDateRule(start, today), resolveDateRule(end, today),
    new DateRangeRules(start, end));
  return [
    option('today', 'Today', currentDay, currentDay),
    option('yesterday', 'Yesterday', yesterday, yesterday),
    option('month-to-date', 'Month to Date', firstDay, currentDay),
    option('this-month', 'This Month', firstDay, lastDay),
    option('previous-month', 'Last Month', {...firstDay, count: 1},
      {...lastDay, count: 1})];
}
