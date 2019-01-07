import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LoginModel } from './login_model';

/** Implements the LoginModel using Nexus service clients. */
export class WebLoginModel extends LoginModel {

  /** Constructs a WebLoginModel.
   * @param serviceClients - The service clients to log into.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this._serviceClients = serviceClients;
    this._account = Beam.DirectoryEntry.INVALID;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public async login(username: string, password: string):
      Promise<Beam.DirectoryEntry> {
    this._account = await this._serviceClients.login(username, password);
    return this._account;
  }

  private _serviceClients: Nexus.ServiceClients;
  private _account: Beam.DirectoryEntry;
}
