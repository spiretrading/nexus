import * as Beam from 'beam';
import { EntitlementsModel } from "./entitlements_model";

/** Implements an EntitlementsModel by accessing web services. */
export class WebEntitlementsModel extends EntitlementsModel {

  /** Constructs a WebEntitlementsModel.
   * @param account - The account to represent.
   */
  constructor(account: Beam.DirectoryEntry) {
    super();
    this._account = account;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public async load(): Promise<void> {
  }

  private _account: Beam.DirectoryEntry;
}
