import * as Beam from 'beam';

/** Interface for a model representing an account's entitlements. */
export abstract class EntitlementsModel {

  /** Returns the account represented. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Returns the current set of entitlements granted to the account. */
  public abstract get entitlements(): Beam.Set<Beam.DirectoryEntry>;

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Submits a request to update the entitlements.
   * @param comment - The comment to include in the request.
   * @param entitlements - The set of entitlements to grant.
   */
  public abstract submit(comment: string,
    entitlements: Beam.Set<Beam.DirectoryEntry>): Promise<void>;
}
