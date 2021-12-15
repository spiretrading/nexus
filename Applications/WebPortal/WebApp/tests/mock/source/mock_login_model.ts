import * as Beam from 'beam';
import * as WebPortal from 'web_portal';

/** Implements the LoginModel with a fixed set of mock accounts. */
export class MockLoginModel extends WebPortal.LoginModel {
  constructor() {
    super();
    this._account = Beam.DirectoryEntry.INVALID;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public async login(username: string, password: string):
      Promise<Beam.DirectoryEntry> {
    this._account = (() => {
      if(username === 'admin') {
        return Beam.DirectoryEntry.makeAccount(110, 'admin');
      } else if(username === 'manager') {
        return Beam.DirectoryEntry.makeAccount(210, 'manager');
      } else if(username === 'trader') {
        return Beam.DirectoryEntry.makeAccount(210, 'trader');
      }
      throw Error('Invalid username or password.');
    })();
    return this._account;
  }

  private _account: Beam.DirectoryEntry;
}
