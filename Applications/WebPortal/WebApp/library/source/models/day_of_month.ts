import { Weekday } from './date_rule';

/** A numeric calendar day or an ordinal day within a month. */
export class DayOfMonth {

  /** The ordinal position, or DAY for a numeric calendar day. */
  public readonly position: DayOfMonth.Position;

  /** The ordinal day type or calendar day, with null for an empty number. */
  public readonly day: Weekday | DayOfMonth.DayType | number;

  /**
   * Constructs a day of the month.
   * @param position - The ordinal position or DAY for a numeric day.
   * @param day - A weekday or day type for an ordinal position, or a number
   *   from 1 through 31 for DAY. Numeric drafts may be null or out of range.
   */
  constructor(position: DayOfMonth.Position,
      day: Weekday | DayOfMonth.DayType | number) {
    this.position = position;
    this.day = day;
  }
}

export namespace DayOfMonth {

  /** The available positions within a month. */
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
    LAST = 'LAST',

    /** A numeric calendar day. */
    DAY = 'DAY'
  }

  /** Day categories that can be selected in addition to specific weekdays. */
  export enum DayType {

    /** Any calendar day. */
    DAY = 'DAY',

    /** A day from Monday through Friday. */
    WEEKDAY = 'WEEKDAY'
  }
}
