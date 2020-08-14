import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { EntitlementsModel } from './entitlements_page';
import { ProfileModel } from './profile_page'
import { RiskModel } from './risk_page';

/** Interface for a model representing a single account. */
export abstract class AccountModel {

  /** The account this model represents. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Returns the account's EntitlementsModel. */
  public abstract get entitlementsModel(): EntitlementsModel;

  /** Returns the account's ProfileModel. */
  public abstract get profileModel(): ProfileModel;

  /** Returns the account's RiskModel. */
  public abstract get riskModel(): RiskModel;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
