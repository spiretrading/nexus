import * as Beam from 'beam';

/** Stores the result of an account query. */
export class AccountQueryResult {

  /** Constructs an AccountQueryResult from a JSON object. */
  public static fromJson(value: any): AccountQueryResult {
    return new AccountQueryResult(
      Beam.DirectoryEntry.fromJson(value.account), value.name);
  }

  /** The account's DirectoryEntry. */
  public readonly account: Beam.DirectoryEntry;

  /** The account's display name. */
  public readonly name: string;

  /**
   * Constructs an AccountQueryResult.
   * @param account The account's DirectoryEntry.
   * @param name The account's display name.
   */
  constructor(account: Beam.DirectoryEntry, name: string) {
    this.account = account;
    this.name = name;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      account: this.account.toJson(),
      name: this.name
    };
  }
}
