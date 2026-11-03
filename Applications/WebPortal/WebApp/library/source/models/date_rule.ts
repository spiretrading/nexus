import * as Beam from 'beam';

/** The supported calendar date transformations. */
export enum DateRuleType {

  /** A fixed calendar date. */
  SPECIFIC_DATE = 'SPECIFIC_DATE',

  /** A calendar day offset. */
  DAY_OFFSET = 'DAY_OFFSET',

  /** A weekday in a relative week. */
  WEEKDAY = 'WEEKDAY',

  /** A numbered day in a relative month. */
  DAY_OF_MONTH = 'DAY_OF_MONTH',

  /** A date near a relative month boundary. */
  MONTH_BOUNDARY = 'MONTH_BOUNDARY',

  /** An ordinal day within a relative month. */
  ORDINAL_DAY_OF_MONTH = 'ORDINAL_DAY_OF_MONTH',
}

/** The days of a calendar week, named as in the C++ API. */
export enum Weekday {

  /** Monday. */
  MONDAY = 'MONDAY',

  /** Tuesday. */
  TUESDAY = 'TUESDAY',

  /** Wednesday. */
  WEDNESDAY = 'WEDNESDAY',

  /** Thursday. */
  THURSDAY = 'THURSDAY',

  /** Friday. */
  FRIDAY = 'FRIDAY',

  /** Saturday. */
  SATURDAY = 'SATURDAY',

  /** Sunday. */
  SUNDAY = 'SUNDAY',
}

/** A fixed calendar date. */
export class SpecificDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.SPECIFIC_DATE;

  /** The fixed date, or null while the input is empty. */
  public readonly date: Beam.Date;

  /** Constructs the rule with its calendar parameters. */
  constructor(date: Beam.Date) {
    this.type = DateRuleType.SPECIFIC_DATE;
    this.date = date;
  }
}

/** A calendar day offset. */
export class DayOffsetDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.DAY_OFFSET;

  /** The signed day offset, or null while the input is empty. */
  public readonly offset: number;

  /** Constructs the rule with its calendar parameters. */
  constructor(offset: number) {
    this.type = DateRuleType.DAY_OFFSET;
    this.offset = offset;
  }
}

/** A weekday in a relative week. */
export class WeekdayDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.WEEKDAY;

  /** The signed week offset, or null while the input is empty. */
  public readonly offset: number;

  /** The selected day in a Monday-based week. */
  public readonly day: Weekday;

  /** Constructs the rule with its calendar parameters. */
  constructor(offset: number, day: Weekday) {
    this.type = DateRuleType.WEEKDAY;
    this.offset = offset;
    this.day = day;
  }
}

/** A numbered day in a relative month. */
export class DayOfMonthDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.DAY_OF_MONTH;

  /** The signed month offset, or null while the input is empty. */
  public readonly offset: number;

  /** The day from 1 to 31, clamped to the month's last day. */
  public readonly day: number;

  /** Constructs the rule with its calendar parameters. */
  constructor(offset: number, day: number) {
    this.type = DateRuleType.DAY_OF_MONTH;
    this.offset = offset;
    this.day = day;
  }
}

/** A date near a relative month boundary. */
export class MonthBoundaryDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.MONTH_BOUNDARY;

  /** The signed month offset, or null while the input is empty. */
  public readonly offset: number;

  /** The selected month boundary. */
  public readonly boundary: MonthBoundaryDateRule.Boundary;

  /** The signed day offset from the boundary, or null while empty. */
  public readonly dayOffset: number;

  /** Constructs the rule with its calendar parameters. */
  constructor(offset: number, boundary: MonthBoundaryDateRule.Boundary,
      dayOffset: number) {
    this.type = DateRuleType.MONTH_BOUNDARY;
    this.offset = offset;
    this.boundary = boundary;
    this.dayOffset = dayOffset;
  }
}

export namespace MonthBoundaryDateRule {

  /** A calendar month's boundary. */
  export enum Boundary {

    /** The first day of the month. */
    FIRST = 'FIRST',

    /** The last day of the month. */
    LAST = 'LAST'
  }
}

/** An ordinal day within a relative calendar month. */
export class OrdinalDayOfMonthDateRule {

  /** The rule discriminator. */
  public readonly type: DateRuleType.ORDINAL_DAY_OF_MONTH;

  /** The signed month offset. */
  public readonly offset: number;

  /** The matching occurrence within the month. */
  public readonly position: OrdinalDayOfMonthDateRule.Position | number;

  /** The calendar days to match. */
  public readonly day: Weekday | OrdinalDayOfMonthDateRule.Day;

  /** Constructs the rule with its calendar parameters. */
  constructor(offset: number,
      position: OrdinalDayOfMonthDateRule.Position | number,
      day: Weekday | OrdinalDayOfMonthDateRule.Day) {
    this.type = DateRuleType.ORDINAL_DAY_OF_MONTH;
    this.offset = offset;
    this.position = position;
    this.day = day;
  }
}

export namespace OrdinalDayOfMonthDateRule {

  /** The matching occurrence within a month. */
  export enum Position {

    /** The first matching day. */
    FIRST = 'FIRST',

    /** The second matching day. */
    SECOND = 'SECOND',

    /** The third matching day. */
    THIRD = 'THIRD',

    /** The fourth matching day. */
    FOURTH = 'FOURTH',

    /** The last matching day. */
    LAST = 'LAST'
  }

  /** Day categories in addition to specific weekdays. */
  export enum Day {

    /** Any calendar day. */
    DAY = 'DAY',

    /** Monday through Friday. */
    WEEKDAY = 'WEEKDAY'
  }
}

/** A calendar rule with only the parameters required by its discriminator. */
export type DateRule = SpecificDateRule | DayOffsetDateRule | WeekdayDateRule |
  DayOfMonthDateRule | MonthBoundaryDateRule | OrdinalDayOfMonthDateRule;

/** Parses the C++ rule representation, with null for an unspecified bound. */
export function parseDateRule(json: any): DateRule {
  if(json == null) {
    return new SpecificDateRule(null);
  }
  const value = json.value;
  let rule: DateRule;
  switch(json.type) {
    case DateRuleType.SPECIFIC_DATE:
      rule = new SpecificDateRule(Beam.Date.fromJson(value.date));
      break;
    case DateRuleType.DAY_OFFSET:
      rule = new DayOffsetDateRule(value.offset);
      break;
    case DateRuleType.WEEKDAY:
      rule = new WeekdayDateRule(value.offset, value.day);
      break;
    case DateRuleType.DAY_OF_MONTH:
      rule = new DayOfMonthDateRule(value.offset, value.day);
      break;
    case DateRuleType.MONTH_BOUNDARY:
      rule = new MonthBoundaryDateRule(value.offset, value.boundary,
        value.day_offset);
      break;
    case DateRuleType.ORDINAL_DAY_OF_MONTH:
      rule = new OrdinalDayOfMonthDateRule(value.offset, (() => {
        if(value.position.type === 'LAST' && value.position.ordinal === 1) {
          return OrdinalDayOfMonthDateRule.Position.LAST;
        }
        if(value.position.type !== 'FIRST') {
          throw new Error('Invalid ordinal position.');
        }
        const ordinal = value.position.ordinal;
        if(Number.isInteger(ordinal) && ordinal >= 1 && ordinal <= 4) {
          return Object.values(OrdinalDayOfMonthDateRule.Position)[ordinal - 1];
        }
        return ordinal;
      })(),
        value.day);
      break;
    default:
      throw new Error('Unknown date rule type.');
  }
  validateRule(rule);
  return rule;
}

/** Encodes a rule using the C++ discriminator and parameter names. */
export function dateRuleToJson(rule: DateRule): any {
  if(rule.type === DateRuleType.SPECIFIC_DATE && rule.date == null) {
    return null;
  }
  validateRule(rule);
  let value: any;
  switch(rule.type) {
    case DateRuleType.SPECIFIC_DATE:
      value = {date: rule.date.toJson()};
      break;
    case DateRuleType.DAY_OFFSET:
      value = {offset: rule.offset};
      break;
    case DateRuleType.WEEKDAY:
    case DateRuleType.DAY_OF_MONTH:
      value = {offset: rule.offset, day: rule.day};
      break;
    case DateRuleType.ORDINAL_DAY_OF_MONTH:
      value = {offset: rule.offset, position: (() => {
        if(rule.position === OrdinalDayOfMonthDateRule.Position.LAST) {
          return {type: 'LAST', ordinal: 1};
        }
        return {type: 'FIRST', ordinal: ordinalPosition(rule.position)};
      })(), day: rule.day};
      break;
    case DateRuleType.MONTH_BOUNDARY:
      value = {offset: rule.offset, boundary: rule.boundary,
        day_offset: rule.dayOffset};
      break;
  }
  return {type: rule.type, value};
}

/**
 * Applies a calendar rule, preserving the reference's time of day.
 * Invalid inputs or results outside years 1400 through 9999 throw an error.
 * The reference is a calendar timestamp in the caller's chosen timezone.
 */
export function applyDateRule(rule: DateRule, reference: Beam.DateTime):
    Beam.DateTime {
  validateRule(rule);
  if(!reference || !isDate(reference.date) || !reference.timeOfDay ||
      !Number.isFinite(reference.timeOfDay.ticks) ||
      reference.timeOfDay.ticks < 0 ||
      reference.timeOfDay.ticks >= Beam.Duration.HOUR.multiply(24).ticks) {
    throw new Error('Invalid reference timestamp.');
  }
  if(rule.type === DateRuleType.SPECIFIC_DATE) {
    return new Beam.DateTime(rule.date, reference.timeOfDay);
  }
  const date = new Date(0);
  date.setUTCFullYear(reference.date.year, reference.date.month - 1,
    reference.date.day);
  if(rule.type === DateRuleType.DAY_OFFSET) {
    date.setUTCDate(date.getUTCDate() + rule.offset);
  } else if(rule.type === DateRuleType.WEEKDAY) {
    const weekday = (date.getUTCDay() + 6) % 7;
    const target = Object.values(Weekday).indexOf(rule.day);
    date.setUTCDate(date.getUTCDate() - weekday + 7 * rule.offset + target);
  } else {
    date.setUTCDate(1);
    date.setUTCMonth(date.getUTCMonth() + rule.offset);
    if(date.getUTCFullYear() < 1400 || date.getUTCFullYear() > 9999 ||
        !Number.isFinite(date.getTime())) {
      throw new RangeError('Date rule month is out of range.');
    }
    date.setUTCMonth(date.getUTCMonth() + 1);
    date.setUTCDate(0);
    if(rule.type === DateRuleType.DAY_OF_MONTH) {
      date.setUTCDate(Math.min(rule.day, date.getUTCDate()));
    } else if(rule.type === DateRuleType.ORDINAL_DAY_OF_MONTH) {
      const days = date.getUTCDate();
      let remaining = ordinalPosition(rule.position);
      let step = 1;
      let number = 1;
      if(rule.position === OrdinalDayOfMonthDateRule.Position.LAST) {
        remaining = 1;
        step = -1;
        number = days;
      }
      const DAYS_PER_WEEK = 7;
      const WEEKDAYS = 5;
      for(; number >= 1 && number <= days; number += step) {
        date.setUTCDate(number);
        const weekday = (date.getUTCDay() + DAYS_PER_WEEK - 1) % DAYS_PER_WEEK;
        if(rule.day === OrdinalDayOfMonthDateRule.Day.DAY ||
            rule.day === OrdinalDayOfMonthDateRule.Day.WEEKDAY &&
              weekday < WEEKDAYS ||
            Object.values(Weekday)[weekday] === rule.day) {
          if(--remaining === 0) {
            break;
          }
        }
      }
      if(remaining !== 0) {
        throw new Error('Ordinal day does not occur in the month.');
      }
    } else {
      if(rule.boundary === MonthBoundaryDateRule.Boundary.FIRST) {
        date.setUTCDate(1);
      }
      date.setUTCDate(date.getUTCDate() + rule.dayOffset);
    }
  }
  const year = date.getUTCFullYear();
  if(!Number.isFinite(date.getTime()) || year < 1400 || year > 9999) {
    throw new RangeError('Date rule result is out of range.');
  }
  return new Beam.DateTime(new Beam.Date(year, date.getUTCMonth() + 1,
    date.getUTCDate()), reference.timeOfDay);
}

/** Resolves an input rule, returning null for an empty or invalid result. */
export function resolveDateRule(rule: DateRule, reference: Beam.Date):
    Beam.Date {
  try {
    return applyDateRule(rule,
      new Beam.DateTime(reference, Beam.Duration.ZERO)).date;
  } catch {
    return null;
  }
}

/** Compares the parameters of two date rules, including incomplete drafts. */
export function isDateRuleEqual(first: DateRule, second: DateRule): boolean {
  if(first.type !== second.type) {
    return false;
  }
  if(first.type === DateRuleType.SPECIFIC_DATE &&
      second.type === DateRuleType.SPECIFIC_DATE) {
    return first.date === second.date ||
      Boolean(first.date?.equals(second.date));
  }
  if(first.type === DateRuleType.SPECIFIC_DATE ||
      second.type === DateRuleType.SPECIFIC_DATE ||
      first.offset !== second.offset) {
    return false;
  }
  if(first.type === DateRuleType.WEEKDAY &&
      second.type === DateRuleType.WEEKDAY ||
      first.type === DateRuleType.DAY_OF_MONTH &&
      second.type === DateRuleType.DAY_OF_MONTH) {
    return first.day === second.day;
  }
  if(first.type === DateRuleType.MONTH_BOUNDARY &&
      second.type === DateRuleType.MONTH_BOUNDARY) {
    return first.boundary === second.boundary &&
      first.dayOffset === second.dayOffset;
  }
  if(first.type === DateRuleType.ORDINAL_DAY_OF_MONTH &&
      second.type === DateRuleType.ORDINAL_DAY_OF_MONTH) {
    return ordinalPosition(first.position) === ordinalPosition(second.position) &&
      first.day === second.day;
  }
  return true;
}

function isDate(date: Beam.Date): boolean {
  if(!date || !Number.isInteger(date.year) || date.year < 1400 ||
      date.year > 9999 || !Number.isInteger(date.month) ||
      !Number.isInteger(date.day)) {
    return false;
  }
  const value = new Date(0);
  value.setUTCFullYear(date.year, date.month - 1, date.day);
  return value.getUTCFullYear() === date.year &&
    value.getUTCMonth() === date.month - 1 && value.getUTCDate() === date.day;
}

function isOffset(value: number): boolean {
  const MINIMUM = -(2 ** 31);
  const MAXIMUM = 2 ** 31 - 1;
  return Number.isInteger(value) && value >= MINIMUM && value <= MAXIMUM;
}

function validateRule(rule: DateRule): void {
  if(rule.type === DateRuleType.SPECIFIC_DATE) {
    if(!isDate(rule.date)) {
      throw new Error('Invalid fixed date.');
    }
    return;
  }
  if(!isOffset(rule.offset)) {
    throw new Error('Invalid date rule offset.');
  }
  if(rule.type === DateRuleType.WEEKDAY) {
    if(!Object.values(Weekday).includes(rule.day)) {
      throw new Error('Invalid weekday.');
    }
  } else if(rule.type === DateRuleType.DAY_OF_MONTH) {
    if(!Number.isInteger(rule.day) || rule.day < 1 || rule.day > 31) {
      throw new Error('Invalid day of month.');
    }
  } else if(rule.type === DateRuleType.MONTH_BOUNDARY) {
    if(!Object.values(MonthBoundaryDateRule.Boundary).includes(rule.boundary) ||
        !isOffset(rule.dayOffset)) {
      throw new Error('Invalid month boundary.');
    }
  } else if(rule.type === DateRuleType.ORDINAL_DAY_OF_MONTH) {
    const ordinal = ordinalPosition(rule.position);
    if((rule.position !== OrdinalDayOfMonthDateRule.Position.LAST &&
        (!Number.isInteger(ordinal) || ordinal < 1 || ordinal > 0xFF)) ||
        ![...Object.values(Weekday),
          ...Object.values(OrdinalDayOfMonthDateRule.Day)].includes(rule.day)) {
      throw new Error('Invalid ordinal day of month.');
    }
  } else if(rule.type !== DateRuleType.DAY_OFFSET) {
    throw new Error('Unknown date rule type.');
  }
}

function ordinalPosition(position: OrdinalDayOfMonthDateRule.Position | number):
    number {
  if(typeof position === 'number') {
    return position;
  }
  if(position === OrdinalDayOfMonthDateRule.Position.LAST) {
    return -1;
  }
  return Object.values(OrdinalDayOfMonthDateRule.Position).indexOf(position) + 1;
}
