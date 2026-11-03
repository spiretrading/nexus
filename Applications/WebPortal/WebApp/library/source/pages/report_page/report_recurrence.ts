import * as Beam from 'beam';
import { applyDateRule, DateRule, DateRuleType, DayOfMonth,
  DayOfMonthDateRule, Interval, OrdinalDayOfMonthDateRule, Weekday,
  WeekdayDateRule } from '../../models';
import { DayOfYear, Recurrence } from './recurrence';

/** The server's recurrence parameters. */
export interface ReportRecurrence {

  /** The calendar interval between occurrences. */
  interval: Interval;

  /** The occurrence within each interval. */
  rule: DateRule;

  /** The selected month for a yearly recurrence. */
  month: number;
}

/** Converts an editable recurrence to the server's calendar rules. */
export function toReportRecurrence(value: Recurrence): ReportRecurrence {
  const interval = value.interval;
  let rule: DateRule = null;
  let month: number = null;
  if(interval.unit === Interval.Unit.WEEK) {
    rule = new WeekdayDateRule(0, value.occurrence as Weekday);
  } else if(interval.unit === Interval.Unit.MONTH ||
      interval.unit === Interval.Unit.YEAR) {
    let day = value.occurrence as DayOfMonth;
    if(interval.unit === Interval.Unit.YEAR) {
      const yearly = value.occurrence as DayOfYear;
      day = yearly.dayOfMonth;
      month = yearly.month;
    }
    if(day.position === DayOfMonth.Position.DAY) {
      rule = new DayOfMonthDateRule(0, day.day as number);
    } else {
      rule = new OrdinalDayOfMonthDateRule(0,
        day.position as unknown as OrdinalDayOfMonthDateRule.Position,
        day.day as Weekday | OrdinalDayOfMonthDateRule.Day);
    }
  }
  return {interval, rule, month};
}

/** Restores the editable occurrence from the server's calendar rules. */
export function fromReportRecurrence(value: ReportRecurrence,
    reference: Beam.Date): Recurrence {
  const {interval, rule, month} = value;
  if(interval.unit === Interval.Unit.DAY) {
    return new Recurrence(interval, null);
  }
  const date = new Date(reference.year, reference.month - 1, reference.day);
  if(interval.unit === Interval.Unit.WEEK) {
    if(rule?.type === DateRuleType.WEEKDAY) {
      return new Recurrence(interval, rule.day);
    }
    return new Recurrence(interval,
      Object.values(Weekday)[(date.getDay() + 6) % 7]);
  }
  let day = new DayOfMonth(DayOfMonth.Position.DAY, reference.day);
  if(rule?.type === DateRuleType.DAY_OF_MONTH) {
    day = new DayOfMonth(DayOfMonth.Position.DAY, rule.day);
  } else if(rule?.type === DateRuleType.ORDINAL_DAY_OF_MONTH) {
    day = new DayOfMonth(rule.position as unknown as DayOfMonth.Position,
      rule.day as Weekday | DayOfMonth.DayType);
  } else if(rule?.type === DateRuleType.MONTH_BOUNDARY) {
    day = new DayOfMonth(rule.boundary as unknown as DayOfMonth.Position,
      DayOfMonth.DayType.DAY);
  }
  if(interval.unit === Interval.Unit.YEAR) {
    return new Recurrence(interval,
      new DayOfYear(day, month ?? reference.month));
  }
  return new Recurrence(interval, day);
}

/**
 * Lists upcoming runs in the browser's schedule timezone.
 * Nonexistent daylight-saving times are skipped and repeated times use the
 * earlier occurrence, matching the reporting service.
 */
export function upcomingReportRuns(start: Beam.DateTime,
    recurrence: ReportRecurrence, now: Date, count: number):
    Beam.DateTime[] {
  if(!start || !now || !recurrence.interval) {
    return [];
  }
  const {interval, rule, month} = recurrence;
  if(!Number.isInteger(interval.count) || interval.count < 1 ||
      interval.count > 0xFFFFFFFF) {
    return [];
  }
  const runs: Beam.DateTime[] = [];
  const current = Beam.Date.fromDate(now);
  const localStart = toLocalDate(start);
  let elapsed = current.year - start.date.year;
  if(interval.unit === Interval.Unit.MONTH) {
    elapsed = elapsed * 12 + current.month - start.date.month;
  } else if(interval.unit === Interval.Unit.DAY ||
      interval.unit === Interval.Unit.WEEK) {
    const MILLISECONDS_PER_DAY = 86400000;
    elapsed = (Date.UTC(current.year, current.month - 1, current.day) -
      Date.UTC(start.date.year, start.date.month - 1, start.date.day)) /
      MILLISECONDS_PER_DAY;
    if(interval.unit === Interval.Unit.WEEK) {
      elapsed = Math.floor(elapsed / 7);
    }
  }
  let step = Math.max(0, Math.floor(elapsed / interval.count) - 1);
  while(runs.length < count) {
    const date = new Date(Date.UTC(start.date.year, start.date.month - 1,
      start.date.day));
    let amount = step++ * interval.count;
    if(interval.unit === Interval.Unit.DAY ||
        interval.unit === Interval.Unit.WEEK) {
      if(interval.unit === Interval.Unit.WEEK) {
        amount *= 7;
      }
      date.setUTCDate(date.getUTCDate() + amount);
    } else {
      if(interval.unit === Interval.Unit.YEAR) {
        amount *= 12;
      }
      date.setUTCDate(1);
      date.setUTCMonth(date.getUTCMonth() + amount);
      if(month != null) {
        date.setUTCMonth(month - 1);
      }
      const last = new Date(Date.UTC(date.getUTCFullYear(),
        date.getUTCMonth() + 1, 0)).getUTCDate();
      date.setUTCDate(Math.min(start.date.day, last));
    }
    if(!Number.isFinite(date.getTime()) || date.getUTCFullYear() > 9999) {
      break;
    }
    let candidate = new Beam.DateTime(new Beam.Date(date.getUTCFullYear(),
      date.getUTCMonth() + 1, date.getUTCDate()), start.timeOfDay);
    if(rule) {
      try {
        candidate = applyDateRule(rule, candidate);
      } catch {
        break;
      }
    }
    const local = toLocalDate(candidate);
    const time = candidate.timeOfDay.split();
    if(local.getFullYear() !== candidate.date.year ||
        local.getMonth() + 1 !== candidate.date.month ||
        local.getDate() !== candidate.date.day ||
        local.getHours() !== time.hours ||
        local.getMinutes() !== time.minutes) {
      continue;
    }
    if(local >= localStart && local > now) {
      runs.push(candidate);
    }
  }
  return runs;
}

function toLocalDate(value: Beam.DateTime): Date {
  const time = value.timeOfDay.split();
  return new Date(value.date.year, value.date.month - 1, value.date.day,
    time.hours, time.minutes, time.seconds);
}
