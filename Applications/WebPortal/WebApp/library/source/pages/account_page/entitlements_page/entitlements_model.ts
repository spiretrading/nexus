import * as Beam from 'beam';

/** Interface for a model representing an account's entitlements. */
export abstract class EntitlementsModel {

  /** Returns the account represented. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
