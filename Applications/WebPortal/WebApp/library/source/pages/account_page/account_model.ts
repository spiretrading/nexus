import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { EntitlementsModel } from './entitlements_page';

/** Interface for a model representing a single account. */
export abstract class AccountModel {

  /** The account this model represents. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Returns a new EntitlementsModel. */
  public abstract makeEntitlementsModel(): EntitlementsModel;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
