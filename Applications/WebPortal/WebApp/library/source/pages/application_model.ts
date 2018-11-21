import * as Beam from 'beam';
import { LoginModel } from './login_page';

/** The top-level application model used to build all sub-models needed by the
 *  application.
 */
export abstract class ApplicationModel {

  /** Returns the account currently logged in. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Returns a new LoginModel. */
  public abstract makeLoginModel(): LoginModel;

  /** Loads the initial application state. */
  public abstract load(): Promise<void>;
}
