import * as Beam from 'beam';

/** A fixed date or a calendar rule evaluated against a reference date. */
export class DateRule {

  /** The calculation used to produce the date. */
  public readonly type: DateRule.Type;

  /** The date used by SPECIFIC_DATE, or null for an unbounded date. */
  public readonly date: Beam.Date;

  /** The number of days, weeks, or months to offset, or null while empty. */
  public readonly count: number;

  /** Whether the offset is before or after the reference period. */
  public readonly direction: DateRule.Direction;

  /** The weekday for WEEKDAY, from Monday (0) through Sunday (6). */
  public readonly weekday: number;

  /** The day number for DAY_OF_MONTH, clamped to the month's last day. */
  public readonly day: number;

  /** The first or last day used by MONTH_BOUNDARY. */
  public readonly boundary: DateRule.Boundary;

  /** The days to offset from a month boundary, or null while empty. */
  public readonly boundaryOffset: number;

  /** Whether the day offset is before or after the month boundary. */
  public readonly boundaryDirection: DateRule.Direction;

  /** Constructs a rule with a zero offset.
   * @param type - The date calculation.
   * @param date - The initial specific date and preferred day of month.
   */
  constructor(type: DateRule.Type, date: Beam.Date) {
    this.type = type;
    this.date = date;
    this.count = 0;
    this.direction = DateRule.Direction.BEFORE;
    this.weekday = 0;
    this.day = date?.day ?? 1;
    this.boundary = DateRule.Boundary.LAST;
    this.boundaryOffset = 0;
    this.boundaryDirection = DateRule.Direction.BEFORE;
  }
}

export namespace DateRule {

  /** The supported date calculations. */
  export enum Type {

    /** A fixed calendar date. */
    SPECIFIC_DATE,

    /** A number of days before or after the reference date. */
    DAY_OFFSET,

    /** A weekday in an offset calendar week. */
    WEEKDAY,

    /** A numbered day in an offset calendar month. */
    DAY_OF_MONTH,

    /** A day offset from the first or last day of a calendar month. */
    MONTH_BOUNDARY
  }

  /** The direction of a calendar offset. */
  export enum Direction {

    /** An earlier date or period. */
    BEFORE,

    /** A later date or period. */
    AFTER
  }

  /** The supported calendar month boundaries. */
  export enum Boundary {

    /** The first day of the month. */
    FIRST,

    /** The last day of the month. */
    LAST
  }
}

/** Resolves a rule, returning null for an empty or invalid result.
 * Weeks begin on Monday. Months clamp numbered days to their last day.
 */
export function resolveDateRule(rule: DateRule, reference: Beam.Date):
    Beam.Date {
  if(rule.type === DateRule.Type.SPECIFIC_DATE) {
    return rule.date;
  }
  if(!reference || rule.count == null || !Number.isSafeInteger(rule.count) ||
      rule.count < 0) {
    return null;
  }
  let offset = rule.count;
  if(rule.direction === DateRule.Direction.BEFORE) {
    offset = -offset;
  }
  const date = new Date(0);
  date.setUTCFullYear(reference.year, reference.month - 1, reference.day);
  if(rule.type === DateRule.Type.DAY_OFFSET) {
    date.setUTCDate(date.getUTCDate() + offset);
  } else if(rule.type === DateRule.Type.WEEKDAY) {
    if(!Number.isInteger(rule.weekday) || rule.weekday < 0 ||
        rule.weekday > 6) {
      return null;
    }
    const weekday = (date.getUTCDay() + 6) % 7;
    date.setUTCDate(date.getUTCDate() - weekday + 7 * offset + rule.weekday);
  } else {
    date.setUTCDate(1);
    date.setUTCMonth(date.getUTCMonth() + offset + 1);
    date.setUTCDate(0);
    if(rule.type === DateRule.Type.DAY_OF_MONTH) {
      if(!Number.isInteger(rule.day) || rule.day < 1 || rule.day > 31) {
        return null;
      }
      date.setUTCDate(Math.min(rule.day, date.getUTCDate()));
    } else if(rule.type === DateRule.Type.MONTH_BOUNDARY) {
      if(rule.boundaryOffset == null ||
          !Number.isSafeInteger(rule.boundaryOffset) ||
          rule.boundaryOffset < 0) {
        return null;
      }
      if(rule.boundary === DateRule.Boundary.FIRST) {
        date.setUTCDate(1);
      }
      let offset = rule.boundaryOffset;
      if(rule.boundaryDirection === DateRule.Direction.BEFORE) {
        offset = -offset;
      }
      date.setUTCDate(date.getUTCDate() + offset);
    } else {
      return null;
    }
  }
  const year = date.getUTCFullYear();
  if(!Number.isFinite(date.getTime()) || year < 0 || year > 9999) {
    return null;
  }
  return new Beam.Date(year, date.getUTCMonth() + 1, date.getUTCDate());
}

/** Compares the configuration of two date rules. */
export function isDateRuleEqual(first: DateRule, second: DateRule): boolean {
  return first.type === second.type && first.count === second.count &&
    first.direction === second.direction && first.weekday === second.weekday &&
    first.day === second.day && first.boundary === second.boundary &&
    first.boundaryOffset === second.boundaryOffset &&
    first.boundaryDirection === second.boundaryDirection &&
    (first.date === second.date || Boolean(first.date?.equals(second.date)));
}
