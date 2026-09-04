import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel } from './create_account_model';

/** Implements an LocalCreateAccountModel in memory. */
export class LocalCreateAccountModel extends CreateAccountModel {

  /** Constructs a LocalCreateAccountModel. */
  public constructor() {
    super();
    this._usernames = [];
  }

  public async createAccount(username: string, groups: Beam.DirectoryEntry, 
      identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles):
      Promise<void> {
    this._usernames.push(CreateAccountModel.normalizeUsername(username));
  }

  public async validateUsername(username: string):
      Promise<CreateAccountModel.ValidationError> {
    const name = CreateAccountModel.normalizeUsername(username);
    const error = CreateAccountModel.checkUsernameFormat(name);
    if(error !== CreateAccountModel.ValidationError.NONE) {
      return error;
    }
    if(this._usernames.indexOf(name) >= 0) {
      return CreateAccountModel.ValidationError.DUPLICATE;
    }
    return CreateAccountModel.ValidationError.NONE;
  }

  private _usernames: string[];
}
