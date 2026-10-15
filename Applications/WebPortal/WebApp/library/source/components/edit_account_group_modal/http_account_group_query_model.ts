import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from './account_group_query_model';

/** Looks up accounts and groups through the administration client. */
export class HttpAccountGroupQueryModel extends AccountGroupQueryModel {
  constructor(client: Nexus.AdministrationClient) {
    super();
    this.client = client;
  }

  public async submit(prefix: string):
      Promise<readonly Beam.DirectoryEntry[]> {
    if(!prefix.trim()) {
      return [];
    }
    const matches = await this.client.searchAccounts(prefix.trim());
    return matches.map(match => match[1]).filter(entry =>
      entry.type === Beam.DirectoryEntry.Type.ACCOUNT ||
      entry.type === Beam.DirectoryEntry.Type.DIRECTORY);
  }

  private client: Nexus.AdministrationClient;
}
