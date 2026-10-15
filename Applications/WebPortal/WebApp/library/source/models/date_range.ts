import * as Beam from 'beam';

/** A calendar date range. Null bounds represent unspecified dates. */
export class DateRange {

  /** The first date, or null for an unspecified start. */
  public readonly start: Beam.Date;

  /** The last date, or null for an unspecified end. */
  public readonly end: Beam.Date;

  /** Constructs a range with the given bounds. */
  constructor(start: Beam.Date, end: Beam.Date) {
    this.start = start;
    this.end = end;
  }
}

/** A named preset offered by a date range input. */
export class DateRangeOption {

  /** The unique option value. The value "custom" is reserved. */
  public readonly value: string;

  /** The text displayed in the selector. */
  public readonly label: string;

  /** The preset's start date, or null for an unspecified start. */
  public readonly startDate: Beam.Date;

  /** The preset's end date, or null for an unspecified end. */
  public readonly endDate: Beam.Date;

  /** Constructs a preset with its identifier, label, and date bounds. */
  constructor(value: string, label: string, startDate: Beam.Date,
      endDate: Beam.Date) {
    this.value = value;
    this.label = label;
    this.startDate = startDate;
    this.endDate = endDate;
  }
}

/** Compares the dates in two ranges, including unspecified bounds. */
export function isDateRangeEqual(first: DateRange, second: DateRange): boolean {
  const equals = (a: Beam.Date, b: Beam.Date) =>
    a === b || a != null && b != null && a.equals(b);
  return equals(first.start, second.start) && equals(first.end, second.end);
}
