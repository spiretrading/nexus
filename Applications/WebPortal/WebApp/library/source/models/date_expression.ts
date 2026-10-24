import * as Beam from 'beam';
import { DayOfMonth } from './day_of_month';
import { DateRule, DayOffsetDateRule, DayOfMonthDateRule,
  OrdinalDayOfMonthDateRule, resolveDateRule,
  SpecificDateRule, Weekday, WeekdayDateRule } from './date_rule';

/** A signed calendar offset, or an incomplete numeric draft. */
export interface DateOffset {

  /** The signed number of units, or null while empty. */
  readonly value: number;

  /** The calendar unit to offset. */
  readonly unit: DateOffset.Unit;
}

export namespace DateOffset {

  /** The supported calendar offset units. */
  export enum Unit {

    /** Calendar days. */
    DAY = 'DAY',

    /** Calendar weeks. */
    WEEK = 'WEEK',

    /** Calendar months. */
    MONTH = 'MONTH'
  }
}

/** A weekday within a relative week. */
export interface DayOfWeekExpression {

  /** The selected weekday. */
  readonly day: Weekday;

  /** The signed offset in weeks. */
  readonly offset: DateOffset;
}

/** An ordinal or numbered day within a relative month. */
export interface DayOfMonthExpression {

  /** The selected day within the month. */
  readonly day: DayOfMonth;

  /** The signed offset in months. */
  readonly offset: DateOffset;
}

/** A fixed or relative date specification, including incomplete drafts. */
export type DateExpression = {

  /** Selects a fixed calendar date. */
  readonly mode: DateExpression.Mode.FIXED;

  /** The calendar date, or null while incomplete. */
  readonly date: Beam.Date;
} | {

  /** Selects a calendar day offset. */
  readonly mode: DateExpression.Mode.DAY_OFFSET;

  /** The signed offset in days. */
  readonly date: DateOffset;
} | {

  /** Selects a weekday in a relative week. */
  readonly mode: DateExpression.Mode.WEEKDAY;

  /** The weekday and week offset. */
  readonly date: DayOfWeekExpression;
} | {

  /** Selects a day in a relative month. */
  readonly mode: DateExpression.Mode.DAY_OF_MONTH;

  /** The day of the month and month offset. */
  readonly date: DayOfMonthExpression;
};

export namespace DateExpression {

  /** The supported date expressions. */
  export enum Mode {

    /** A fixed calendar date. */
    FIXED = 'FIXED',

    /** A calendar day offset. */
    DAY_OFFSET = 'DAY_OFFSET',

    /** A weekday in a relative week. */
    WEEKDAY = 'WEEKDAY',

    /** A day in a relative month. */
    DAY_OF_MONTH = 'DAY_OF_MONTH'
  }
}

/** Resolves an expression, returning null for an empty or invalid value. */
export function resolveDateExpression(expression: DateExpression,
    reference: Beam.Date): Beam.Date {
  return resolveDateRule(dateExpressionToRule(expression), reference);
}

/** Converts an editable expression to its serializable calendar rule. */
export function dateExpressionToRule(expression: DateExpression): DateRule {
  if(expression.mode === DateExpression.Mode.FIXED) {
    return new SpecificDateRule(expression.date);
  } else if(expression.mode === DateExpression.Mode.DAY_OFFSET) {
    return new DayOffsetDateRule(expression.date.value);
  } else if(expression.mode === DateExpression.Mode.WEEKDAY) {
    return new WeekdayDateRule(expression.date.offset.value,
      expression.date.day);
  }
  const {day, offset} = expression.date;
  if(day.position === DayOfMonth.Position.DAY) {
    return new DayOfMonthDateRule(offset.value, day.day as number);
  }
  return new OrdinalDayOfMonthDateRule(offset.value,
    day.position as unknown as OrdinalDayOfMonthDateRule.Position,
    day.day as Weekday | OrdinalDayOfMonthDateRule.Day);
}

/** Compares expression parameters, including incomplete drafts. */
export function isDateExpressionEqual(first: DateExpression,
    second: DateExpression): boolean {
  if(first === second) {
    return true;
  }
  if(!first || !second || first.mode !== second.mode) {
    return false;
  }
  if(first.mode === DateExpression.Mode.FIXED &&
      second.mode === DateExpression.Mode.FIXED) {
    return first.date === second.date || !!first.date?.equals(second.date);
  } else if(first.mode === DateExpression.Mode.DAY_OFFSET &&
      second.mode === DateExpression.Mode.DAY_OFFSET) {
    return first.date.value === second.date.value;
  } else if(first.mode === DateExpression.Mode.WEEKDAY &&
      second.mode === DateExpression.Mode.WEEKDAY) {
    return first.date.day === second.date.day &&
      first.date.offset.value === second.date.offset.value;
  } else if(first.mode === DateExpression.Mode.DAY_OF_MONTH &&
      second.mode === DateExpression.Mode.DAY_OF_MONTH) {
    return first.date.day.position === second.date.day.position &&
      first.date.day.day === second.date.day.day &&
      first.date.offset.value === second.date.offset.value;
  }
  return false;
}
