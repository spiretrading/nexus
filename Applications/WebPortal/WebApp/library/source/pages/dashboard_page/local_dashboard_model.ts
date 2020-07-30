import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel, LocalAccountModel } from '..';
import { DashboardModel } from './dashboard_model';

/** Implements the DashboardModel using local memory. */
export class LocalDashboardModel extends DashboardModel {

  /** Constructs a LocalDashboardModel.
   * @param account - The account that's logged in.
   * @param roles - The account's roles.
   * @param groups - The account's groups.
   * @param entitlementDatabase - The entitlement database to use.
   * @param countryDatabase - The country database to use.
   * @param currencyDatabase - The currency database to use.
   * @param marketDatabase - The market database to use.
   * @param accountDirectoryModel - The AccountDirectoryModel to return.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      groups: Beam.DirectoryEntry[],
      entitlementDatabase: Nexus.EntitlementDatabase,
      countryDatabase: Nexus.CountryDatabase,
      currencyDatabase: Nexus.CurrencyDatabase,
      marketDatabase: Nexus.MarketDatabase,
      accountDirectoryModel: AccountDirectoryModel) {
    super();
    this._isLoaded = false;
    this._entitlementDatabase = entitlementDatabase;
    this._countryDatabase = countryDatabase;
    this._currencyDatabase = currencyDatabase;
    this._marketDatabase = marketDatabase;
    this._account = account;
    this._roles = roles;
    this._groups = groups;
    this._accountDirectoryModel = accountDirectoryModel;
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get entitlementDatabase(): Nexus.EntitlementDatabase {
    return this._entitlementDatabase;
  }

  public get countryDatabase(): Nexus.CountryDatabase {
    return this._countryDatabase;
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

  public get groups(): Beam.DirectoryEntry[] {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._groups;
  }

  public get accountDirectoryModel(): AccountDirectoryModel {
    return this._accountDirectoryModel;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): LocalAccountModel {
    if(account.equals(this._account)) {
      return new LocalAccountModel(this.account, this.roles, this.groups);
    }
    return new LocalAccountModel(account, new Nexus.AccountRoles(0), []);
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async logout(): Promise<void> {
    return;
  }

  private _isLoaded: boolean;
  private _entitlementDatabase: Nexus.EntitlementDatabase;
  private _countryDatabase: Nexus.CountryDatabase;
  private _currencyDatabase: Nexus.CurrencyDatabase;
  private _marketDatabase: Nexus.MarketDatabase;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
  private _groups: Beam.DirectoryEntry[];
  private _accountDirectoryModel: AccountDirectoryModel;
}
