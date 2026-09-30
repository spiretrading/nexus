/** The reason an input value failed validation. */
export enum ValidationError {

  /** No validation error. */
  NONE,

  /** A required value is empty. */
  REQUIRED,

  /** The value conflicts with an existing value. */
  DUPLICATE,

  /** The value is present but malformed. */
  FORMAT,

  /** The date or time is in the past. */
  PAST_DATE,

  /** The end value is less than the start value. */
  OUT_OF_RANGE
}
