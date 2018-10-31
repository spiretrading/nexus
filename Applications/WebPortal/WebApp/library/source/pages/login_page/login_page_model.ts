import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Provides the connectivity needed by the LoginPage to login a user. */
export abstract class LoginPageModel {

  /** Returns the account currently logged in. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Logs into the client web portal.
   * @param username - The username.
   * @param password - The password.
   * @return The directory entry of the account that logged in.
   * @throws ServiceError - The login service request failed.
   */
  public abstract async login(username: string, password: string):
    Promise<Beam.DirectoryEntry>;
}
