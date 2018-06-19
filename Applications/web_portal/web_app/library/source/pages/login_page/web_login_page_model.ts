import * as Beam from 'beam';
import * as Nexus from 'nexus';
import {LoginPageModel} from './login_page_model';

/** Implements the LoginPageModel using Nexus service clients. */
export class WebLoginPageModel extends LoginPageModel {

  /** Constructs a WebLoginPageModel.
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
