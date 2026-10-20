import * as Beam from 'beam';
import { DayOfMonth } from './day_of_month';
import { Weekday } from './date_rule';

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
