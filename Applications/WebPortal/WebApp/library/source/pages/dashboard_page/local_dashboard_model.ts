import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalAccountModel } from '..';
import { DashboardModel } from './dashboard_model';

/** Implements the DashboardModel using local memory. */
export class LocalDashboardModel extends DashboardModel {

  /** Constructs a LocalDashboardModel.
   * @param account - The account that's logged in.
   * @param roles - The account's roles.
   * @param entitlementDatabase - The entitlement database to use.
   * @param currencyDatabase - The currency database to use.
   * @param marketDatabase - The market database to use.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      entitlementDatabase: Nexus.EntitlementDatabase,
      currencyDatabase: Nexus.CurrencyDatabase,
      marketDatabase: Nexus.MarketDatabase) {
    super();
    this._isLoaded = false;
    this._entitlementDatabase = entitlementDatabase;
    this._currencyDatabase = currencyDatabase;
    this._marketDatabase = marketDatabase;
    this._account = account;
    this._roles = roles;
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get entitlementDatabase(): Nexus.EntitlementDatabase {
    return this._entitlementDatabase;
  }

  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this._currencyDatabase;
  }

  public get marketDatabase(): Nexus.MarketDatabase {
    return this._marketDatabase;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._roles;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): LocalAccountModel {
    if(account.equals(this._account)) {
      return new LocalAccountModel(this.account, this.roles);
    }
    return new LocalAccountModel(account, new Nexus.AccountRoles(0));
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async logout(): Promise<void> {
    return;
  }

  private _isLoaded: boolean;
  private _entitlementDatabase: Nexus.EntitlementDatabase;
  private _currencyDatabase: Nexus.CurrencyDatabase;
  private _marketDatabase: Nexus.MarketDatabase;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
