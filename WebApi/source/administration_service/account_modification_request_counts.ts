/** Stores the number of account modification requests in each state. */
export class AccountModificationRequestCounts {

  /** Constructs an AccountModificationRequestCounts from a JSON object. */
  public static fromJson(value: any): AccountModificationRequestCounts {
    return new AccountModificationRequestCounts(
      value.pending, value.granted, value.rejected);
  }

  /**
   * Constructs an AccountModificationRequestCounts.
   * @param pending - The number of requests awaiting a decision.
   * @param granted - The number of requests that have been granted.
   * @param rejected - The number of requests that have been rejected.
   */
  constructor(pending: number, granted: number, rejected: number) {
    this._pending = pending;
    this._granted = granted;
    this._rejected = rejected;
  }

  /** Returns the number of requests awaiting a decision. */
  public get pending(): number {
    return this._pending;
  }

  /** Returns the number of requests that have been granted. */
  public get granted(): number {
    return this._granted;
  }

  /** Returns the number of requests that have been rejected. */
  public get rejected(): number {
    return this._rejected;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      pending: this._pending,
      granted: this._granted,
      rejected: this._rejected
    };
  }

  private _pending: number;
  private _granted: number;
  private _rejected: number;
}
