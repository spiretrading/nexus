import * as Beam from 'beam';
import { LoginModel } from './login_page';

/** The top-level application model used to build all sub-models needed by the
 *  application.
 */
export abstract class ApplicationModel {

  /** Returns a new LoginModel. */
  public abstract makeLoginModel(): LoginModel;

  /** Loads the account. */
  public abstract async loadAccount(): Promise<Beam.DirectoryEntry>;
}
