import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel, ComplianceModel, LocalAccountModel, LocalGroupModel } from '..';
import { DashboardModel } from './dashboard_model';

/** Implements the DashboardModel using local memory. */
export class LocalDashboardModel extends DashboardModel {

  /**
   * Constructs a LocalDashboardModel.
   * @param account - The account that's logged in.
   * @param roles - The account's roles.
   * @param entitlementDatabase - The entitlement database to use.
   * @param countryDatabase - The country database to use.
   * @param currencyDatabase - The currency database to use.
   * @param venueDatabase - The venue database to use.
   * @param accountDirectoryModel - The AccountDirectoryModel to return.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      entitlementDatabase: Nexus.EntitlementDatabase,
      countryDatabase: Nexus.CountryDatabase,
      currencyDatabase: Nexus.CurrencyDatabase,
      venueDatabase: Nexus.VenueDatabase,
      accountDirectoryModel: AccountDirectoryModel) {
    super();
    this._isLoaded = false;
    this._entitlementDatabase = entitlementDatabase;
    this._countryDatabase = countryDatabase;
    this._currencyDatabase = currencyDatabase;
    this._venueDatabase = venueDatabase;
    this._account = account;
    this._roles = roles;
    this._accountDirectoryModel = accountDirectoryModel;
    this.accountModels = new Beam.Map<Beam.DirectoryEntry, LocalAccountModel>();
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get entitlementDatabase(): Nexus.EntitlementDatabase {
    this.ensureLoaded();
    return this._entitlementDatabase;
  }

  public get countryDatabase(): Nexus.CountryDatabase {
    this.ensureLoaded();
    return this._countryDatabase;
  }

  public get currencyDatabase(): Nexus.CurrencyDatabase {
    this.ensureLoaded();
    return this._currencyDatabase;
  }

  public get venueDatabase(): Nexus.VenueDatabase {
    this.ensureLoaded();
    return this._venueDatabase;
  }

  public get account(): Beam.DirectoryEntry {
    this.ensureLoaded();
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    this.ensureLoaded();
    return this._roles;
  }

  public get accountDirectoryModel(): AccountDirectoryModel {
    this.ensureLoaded();
    return this._accountDirectoryModel;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): LocalAccountModel {
    this.ensureLoaded();
    let model = this.accountModels.get(account);
    if(model === undefined) {
      model = (() => {
        if(account.equals(this._account)) {
          return new LocalAccountModel(this.account, this.roles, [],
            new ComplianceModel(this.account, [], [], this._currencyDatabase));
        }
        return new LocalAccountModel(account, new Nexus.AccountRoles(0), [],
          new ComplianceModel(account, [], [], this._currencyDatabase));
      })();
      this.accountModels.set(account, model);
    }
    return model;
  }

  public makeGroupModel(group: Beam.DirectoryEntry): LocalGroupModel {
    this.ensureLoaded();
    return new LocalGroupModel(
      group, [], new ComplianceModel(group, [], [], this._currencyDatabase));
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async logout(): Promise<void> {
    return;
  }

  private ensureLoaded() {
    if(!this._isLoaded) {
      throw Error('Model not loaded.');
    }
  }

  private _isLoaded: boolean;
  private _entitlementDatabase: Nexus.EntitlementDatabase;
  private _countryDatabase: Nexus.CountryDatabase;
  private _currencyDatabase: Nexus.CurrencyDatabase;
  private _venueDatabase: Nexus.VenueDatabase;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
  private _accountDirectoryModel: AccountDirectoryModel;
  private accountModels: Beam.Map<Beam.DirectoryEntry, LocalAccountModel>;
}
