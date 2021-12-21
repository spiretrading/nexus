import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ComplianceService } from './compliance_page';
import { EntitlementsModel } from './entitlements_page';
import { ProfileModel } from './profile_page'
import { RiskModel } from './risk_page';

/** Interface for a model representing a single account. */
export abstract class AccountModel {

  /** The account this model represents. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** The trading groups the account is a member of. */
  public abstract get groups(): Beam.DirectoryEntry[];

  /** Returns the account's EntitlementsModel. */
  public abstract get entitlementsModel(): EntitlementsModel;

  /** Returns the account's ProfileModel. */
  public abstract get profileModel(): ProfileModel;

  /** Returns the account's RiskModel. */
  public abstract get riskModel(): RiskModel;

  /** Returns the ComplianceService tied to the account. */
  public abstract get complianceService(): ComplianceService;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
