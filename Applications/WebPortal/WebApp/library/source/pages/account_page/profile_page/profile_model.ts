import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Interface for a model representing an account's profile. */
export abstract class ProfileModel {

  /** Returns the account represented. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Returns the account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Returns the account's identity. */
  public abstract get identity(): Nexus.AccountIdentity;

  /** Returns the account's readonly condition. */
  public abstract get isReadonly(): boolean;

  /** Loads this model. */
  public abstract async load(): Promise<void>;

  /** Updates the account's identity.
   * @param roles - The update roles.
   * @param identity - The updated identity.
   */
  public abstract async updateIdentity(roles: Nexus.AccountRoles,
    identity: Nexus.AccountIdentity): Promise<void>;

  /** Updates the account's password.
   * @param password - The updated password.
   */
  public abstract async updatePassword(password: string): Promise<void>;
}
