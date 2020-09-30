import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { LocalEntitlementsModel } from './entitlements_page';
import { LocalProfileModel } from './profile_page';
import { LocalRiskModel } from './risk_page';

/** Implements an in-memory AccountModel. */
export class LocalAccountModel extends AccountModel {

  /** Constructs a LocalAccountModel. */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      groups: Beam.DirectoryEntry[]) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._roles = roles;
    this._groups = groups.slice();
    this._entitlementsModel = new LocalEntitlementsModel(this._account,
      new Beam.Set<Beam.DirectoryEntry>());
    this._profileModel = new LocalProfileModel(this._account, this._roles, 
      new Nexus.AccountIdentity());
    this._riskModel = new LocalRiskModel(this._account,
      Nexus.RiskParameters.INVALID);
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
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
    return this._groups.slice();
  }

  public get entitlementsModel(): LocalEntitlementsModel {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._entitlementsModel;
  }

  public get profileModel(): LocalProfileModel {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._profileModel;
  }

  public get riskModel(): LocalRiskModel {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._riskModel;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
  private _groups: Beam.DirectoryEntry[];
  private _entitlementsModel: LocalEntitlementsModel;
  private _profileModel: LocalProfileModel;
  private _riskModel: LocalRiskModel;
}
