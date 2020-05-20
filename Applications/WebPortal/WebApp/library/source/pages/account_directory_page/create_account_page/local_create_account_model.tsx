import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel } from './create_account_model';

/** Implements an LocalCreateAccountModel in memory. */
export class LocalCreateAccountModel extends CreateAccountModel {

  public async createAccount(username: string, groups: Beam.DirectoryEntry, 
    identity: Nexus.AccountIdentity, 
    roles: Nexus.AccountRoles): Promise<void> {}
}
