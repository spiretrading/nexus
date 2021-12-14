import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { ComplianceModel, ComplianceService, LocalComplianceService } from
  './compliance_page';
import { LocalEntitlementsModel } from './entitlements_page';
import { LocalProfileModel } from './profile_page';
import { LocalRiskModel } from './risk_page';

/** Implements an in-memory AccountModel. */
export class LocalAccountModel extends AccountModel {

  /** Constructs a LocalAccountModel. */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      groups: Beam.DirectoryEntry[], complianceModel: ComplianceModel) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._roles = roles;
    this._groups = groups.slice();
    this._entitlementsModel = new LocalEntitlementsModel(
      this._account, new Beam.Set<Beam.DirectoryEntry>());
    this._profileModel = new LocalProfileModel(
      this._account, this._roles,  new Nexus.AccountIdentity());
    this._riskModel =
      new LocalRiskModel(this._account, Nexus.RiskParameters.INVALID);
    this._complianceService = new LocalComplianceService(complianceModel);
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    this.ensureLoaded();
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    this.ensureLoaded();
    return this._roles;
  }

  public get groups(): Beam.DirectoryEntry[] {
    this.ensureLoaded();
    return this._groups.slice();
  }

  public get entitlementsModel(): LocalEntitlementsModel {
    this.ensureLoaded();
    return this._entitlementsModel;
  }

  public get profileModel(): LocalProfileModel {
    this.ensureLoaded();
    return this._profileModel;
  }

  public get riskModel(): LocalRiskModel {
    this.ensureLoaded();
    return this._riskModel;
  }

  public get complianceService(): ComplianceService {
    this.ensureLoaded();
    return this._complianceService;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
  private _groups: Beam.DirectoryEntry[];
  private _entitlementsModel: LocalEntitlementsModel;
  private _profileModel: LocalProfileModel;
  private _riskModel: LocalRiskModel;
  private _complianceService: LocalComplianceService;
}
