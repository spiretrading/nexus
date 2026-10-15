/** A number of calendar units. A null count represents an empty input. */
export class Interval {

  /** The number of units, or null while the count is empty. */
  public readonly count: number;

  /** The calendar unit. */
  public readonly unit: Interval.Unit;

  /** Constructs a calendar interval.
   * @param count - The number of units, or null for an empty count.
   * @param unit - The calendar unit used by the count.
   */
  constructor(count: number, unit: Interval.Unit) {
    this.count = count;
    this.unit = unit;
  }
}

export namespace Interval {

  /** The supported calendar units. */
  export enum Unit {

    /** A calendar day. */
    DAY,

    /** A calendar week. */
    WEEK,

    /** A calendar month. */
    MONTH,

    /** A calendar year. */
    YEAR
  }
}
