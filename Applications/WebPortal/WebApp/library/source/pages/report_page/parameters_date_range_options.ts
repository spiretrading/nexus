import * as Beam from 'beam';
import { DateRangeOption, DateRangeRules } from '../../models/date_range';
import { DateRule, DayOffsetDateRule, MonthBoundaryDateRule,
  resolveDateRule } from '../../models/date_rule';

/**
 * Builds the reporting presets relative to a local calendar date.
 * @param today - The reference date for the presets.
 * @return The reporting presets in display order, excluding Custom.
 */
export function makeParametersDateRangeOptions(today: Beam.Date):
    DateRangeOption[] {
  const currentDay = new DayOffsetDateRule(0);
  const yesterday = new DayOffsetDateRule(-1);
  const firstDay =
    new MonthBoundaryDateRule(0, MonthBoundaryDateRule.Boundary.FIRST, 0);
  const lastDay =
    new MonthBoundaryDateRule(0, MonthBoundaryDateRule.Boundary.LAST, 0);
  const option = (value: string, label: string, start: DateRule,
      end: DateRule) => new DateRangeOption(value, label,
    resolveDateRule(start, today), resolveDateRule(end, today),
    new DateRangeRules(start, end));
  return [
    option('today', 'Today', currentDay, currentDay),
    option('yesterday', 'Yesterday', yesterday, yesterday),
    option('month-to-date', 'Month to Date', firstDay, currentDay),
    option('this-month', 'This Month', firstDay, lastDay),
    option('previous-month', 'Last Month',
      new MonthBoundaryDateRule(-1, MonthBoundaryDateRule.Boundary.FIRST, 0),
      new MonthBoundaryDateRule(-1, MonthBoundaryDateRule.Boundary.LAST, 0))];
}
