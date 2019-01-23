import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Represents an account entry and its roles. */
export class AccountEntry {

  /** Constructs an AccountEntry
   * @param account - The account to represent.
   * @param roles - The roles associated with the account.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    this._account = account;
    this._roles = roles;
  }

  /** Returns the directory entry associated with the account. */
  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  /** Returns the roles associated with the account. */
  public get roles(): Nexus.AccountRoles {
    return this._roles;
  }

  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
