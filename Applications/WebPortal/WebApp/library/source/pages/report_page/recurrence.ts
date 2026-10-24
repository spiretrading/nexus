import * as Beam from 'beam';
import { DayOfMonth, Interval, Weekday } from '../../models';

/** A day within a particular month of the year. */
export class DayOfYear {

  /** The day within the selected month. */
  public readonly dayOfMonth: DayOfMonth;

  /** The month of the year. */
  public readonly month: Beam.Date.Month;

  /** Constructs a yearly occurrence from its day and month. */
  constructor(dayOfMonth: DayOfMonth, month: Beam.Date.Month) {
    this.dayOfMonth = dayOfMonth;
    this.month = month;
  }
}

/** A recurring interval and the occurrence within it. */
export class Recurrence {

  /** The number and unit of calendar intervals between occurrences. */
  public readonly interval: Interval;

  /**
   * The weekday, day of month, or day of year for the interval's unit.
   * Daily recurrences have a null occurrence.
   */
  public readonly occurrence: Weekday | DayOfMonth | DayOfYear;

  /** Constructs a recurrence with an occurrence matching its interval unit. */
  constructor(interval: Interval,
      occurrence: Weekday | DayOfMonth | DayOfYear) {
    this.interval = interval;
    this.occurrence = occurrence;
  }
}
