import * as Beam from 'beam';

/** Represents a request to modify an account's entitlements. */
export class EntitlementModification {

  /** Constructs an EntitlementModification from a JSON object. */
  public static fromJson(value: any): EntitlementModification {
    return new EntitlementModification(Beam.Set.fromJson(
      Beam.DirectoryEntry, value.entitlements));
  }

  /**
   * Constructs an EntitlementModification.
   * @param entitlements - The set of entitlements to grant.
   */
  constructor(entitlements: Beam.Set<Beam.DirectoryEntry>) {
    this._entitlements = entitlements.clone();
  }

  /** Returns the set of entitlements to grant. */
  public get entitlements(): Beam.Set<Beam.DirectoryEntry> {
    return this._entitlements;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      entitlements: Beam.toJson(this.entitlements)
    };
  }

  private _entitlements: Beam.Set<Beam.DirectoryEntry>;
}
