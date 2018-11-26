import * as Beam from 'beam';
import { EntitlementsModel } from "./entitlements_model";

/** Implements an EntitlementsModel in memory. */
export class LocalEntitlementsModel extends EntitlementsModel {

  /** Constructs an empty model.
   * @param account - The account to represent.
   */
  constructor(account: Beam.DirectoryEntry) {
    super();
    this._isLoaded = false;
    this._account = account;
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
}
