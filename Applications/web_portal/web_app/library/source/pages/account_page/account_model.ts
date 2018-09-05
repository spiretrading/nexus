import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Interface for a model representing a single account. */
export abstract class AccountModel {

  /** The account this model represents. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
