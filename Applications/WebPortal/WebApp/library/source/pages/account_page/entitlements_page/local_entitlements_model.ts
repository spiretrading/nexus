import * as Beam from 'beam';
import { EntitlementsModel } from "./entitlements_model";

/** Implements an EntitlementsModel in memory. */
export class LocalEntitlementsModel extends EntitlementsModel {

  /** Constructs an empty model.
   * @param account - The account to represent.
   * @param entitlements - The set of entitlements granted to the account.
   */
  constructor(account: Beam.DirectoryEntry,
      entitlements: Beam.Set<Beam.DirectoryEntry>) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._entitlements = entitlements.clone();
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get entitlements(): Beam.Set<Beam.DirectoryEntry> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._entitlements.clone();
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async submit(comment: string,
      entitlements: Beam.Set<Beam.DirectoryEntry>): Promise<void> {
    return;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _entitlements: Beam.Set<Beam.DirectoryEntry>;
}
