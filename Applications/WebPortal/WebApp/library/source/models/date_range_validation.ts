import * as Beam from 'beam';
import { DateRange } from './date_range';

/** The validity and error presentation of a date range. */
export class DateRangeValidation {

  /** Whether the range can be committed. */
  public readonly valid: boolean;

  /** The reason the range is invalid. */
  public readonly error: DateRangeValidation.Error;

  /** The bounds associated with the error. */
  public readonly target: DateRangeValidation.Target;

  /** Whether the error should be displayed. */
  public readonly showError: boolean;

  /** Constructs validation with its error, affected bounds, and visibility. */
  constructor(error: DateRangeValidation.Error,
      target: DateRangeValidation.Target, showError: boolean) {
    this.valid = error === DateRangeValidation.Error.NONE;
    this.error = error;
    this.target = target;
    this.showError = showError && !this.valid;
  }
}

export namespace DateRangeValidation {

  /** The errors produced by a date range input. */
  export enum Error {

    /** The range is valid. */
    NONE,

    /** A required date is missing. */
    REQUIRED,

    /** A date is incomplete or malformed. */
    FORMAT,

    /** The end date precedes the start date. */
    OUT_OF_RANGE
  }

  /** The bounds associated with a validation error. */
  export enum Target {

    /** Neither bound. */
    NONE,

    /** The start date. */
    START,

    /** The end date. */
    END,

    /** Both dates. */
    START_AND_END
  }
}

/** Validates a range's ordering and required bounds.
 * @param value - The range to validate.
 * @param boundsRequired - Whether both dates must be specified.
 * @return Validation with errors initially hidden.
 */
export function validateDateRange(value: DateRange, boundsRequired: boolean):
    DateRangeValidation {
  const {Error, Target} = DateRangeValidation;
  const startValid = isValidDate(value.start);
  const endValid = isValidDate(value.end);
  if(!startValid || !endValid) {
    let target = Target.START_AND_END;
    if(startValid) {
      target = Target.END;
    } else if(endValid) {
      target = Target.START;
    }
    return new DateRangeValidation(Error.FORMAT, target, false);
  }
  if(value.start && value.end && value.start.compare(value.end) > 0) {
    return new DateRangeValidation(Error.OUT_OF_RANGE, Target.END, false);
  }
  if(boundsRequired) {
    if(!value.start && !value.end) {
      return new DateRangeValidation(Error.REQUIRED, Target.START_AND_END,
        false);
    }
    if(!value.start) {
      return new DateRangeValidation(Error.REQUIRED, Target.START, false);
    }
    if(!value.end) {
      return new DateRangeValidation(Error.REQUIRED, Target.END, false);
    }
  }
  return new DateRangeValidation(Error.NONE, Target.NONE, false);
}

function isValidDate(value: Beam.Date): boolean {
  if(value == null) {
    return true;
  }
  const date = new Date(0);
  date.setUTCFullYear(value.year, value.month - 1, value.day);
  return value.year >= 0 && value.year <= 9999 &&
    date.getUTCFullYear() === value.year &&
    date.getUTCMonth() === value.month - 1 && date.getUTCDate() === value.day;
}
