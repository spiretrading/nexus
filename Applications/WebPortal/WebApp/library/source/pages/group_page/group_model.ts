import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Interface for a model representing a single account. */
export abstract class GroupModel {

  /** The account this model represents. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** The trading groups the account is a member of. */
  public abstract get groups(): Beam.DirectoryEntry[];

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
