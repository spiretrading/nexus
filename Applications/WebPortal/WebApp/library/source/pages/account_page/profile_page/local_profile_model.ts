import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ProfileModel } from './profile_model';

/** Implements the ProfileModel in-memory. */
export class LocalProfileModel extends ProfileModel {

  /** Constructs a model.
   * @param account - The account to represent.
   * @param roles - The account's roles.
   * @param identity - The account's identity.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      identity: Nexus.AccountIdentity) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._roles = roles;
    this._identity = identity.clone();
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

  public get roles(): Nexus.AccountRoles {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._roles.clone();
  }

  public get groups(): Beam.DirectoryEntry[] {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._groups.slice();
  }

  public set groups(groups: Beam.DirectoryEntry[]) {
    this._groups = groups.slice();
  }

  public get identity(): Nexus.AccountIdentity {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._identity.clone();
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async updateIdentity(roles: Nexus.AccountRoles,
      identity: Nexus.AccountIdentity): Promise<void> {
    this._roles = roles;
    this._identity = identity.clone();
  }

  public async updatePassword(password: string): Promise<void> {}

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
  private _groups: Beam.DirectoryEntry[];
  private _identity: Nexus.AccountIdentity;
}
