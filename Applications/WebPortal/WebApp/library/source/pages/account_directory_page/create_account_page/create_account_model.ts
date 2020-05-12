import * as Beam from 'beam';
import * as Nexus from 'nexus';

export abstract class CreateAccountModel {

  /** Loads this model. */
  public abstract async load(): Promise<void>;

  /** Submits information to create an account.
    @param username - The username of the new account.
    @param groups - The groups associated with the new account.
    @param identity - The identity of the new account.
    @param roles - The roles associated with the new account.
   */
  public abstract async onSubmitNewAccount(username: string, 
    groups: Beam.DirectoryEntry[], identity: Nexus.AccountIdentity,
    roles: Nexus.AccountRoles): Promise<void>;
}