import * as Beam from 'beam';
import { LoginModel } from './login_model';

/** Implements the LoginModel for testing purposes. */
export class LocalLoginModel extends LoginModel {

  /** Accepts the username and password. */
  public accept(): void {
    this.loginResolver(this.account);
  }

  /**
   * Rejects the login.
   * @param message - The reason for the rejection.
   */
  public reject(message: string): void {
    this.loginRejector(new Beam.ServiceError(message));
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public async login(username: string, password: string):
      Promise<Beam.DirectoryEntry> {
    return new Promise<Beam.DirectoryEntry>((resolve, reject) => {
      this._account = Beam.DirectoryEntry.makeAccount(123, username);
      this.loginResolver = resolve;
      this.loginRejector = reject;
    });
  }

  private _account: Beam.DirectoryEntry;
  private loginResolver: (account: Beam.DirectoryEntry) => void;
  private loginRejector: (error: Beam.ServiceError) => void;
}
