import * as Beam from 'beam';
import * as Nexus from 'nexus';

export class AccountEntry {

  /** Constructs an AccountEntry
   * @param account - The account to represent.
   * @param roles - The roles associcated with the account.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    this._account = account;
    this._roles = roles;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    return this._roles;
  }

  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
