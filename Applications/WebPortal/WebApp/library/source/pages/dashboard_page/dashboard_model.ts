import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Base class for the model used by the DashboardPage. */
export abstract class DashboardModel {

  /** The account currently logged in. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Loads this model. */
  public abstract async load(): Promise<void>;

  /** Logs the user out. */
  public abstract async logout(): Promise<void>;
}
