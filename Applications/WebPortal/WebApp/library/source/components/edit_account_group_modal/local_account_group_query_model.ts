import * as Beam from 'beam';
import { AccountGroupQueryModel } from './account_group_query_model';

/** Looks up accounts and groups in a local collection. */
export class LocalAccountGroupQueryModel extends AccountGroupQueryModel {
  constructor(entries: readonly Beam.DirectoryEntry[]) {
    super();
    this.entries = entries.filter(entry =>
      entry.type === Beam.DirectoryEntry.Type.ACCOUNT ||
      entry.type === Beam.DirectoryEntry.Type.DIRECTORY);
  }

  public async submit(prefix: string): Promise<readonly Beam.DirectoryEntry[]> {
    const query = prefix.trim().toLocaleLowerCase();
    if(!query) {
      return [];
    }
    return this.entries.filter(
      entry => entry.name.toLocaleLowerCase().startsWith(query));
  }

  private entries: readonly Beam.DirectoryEntry[];
}
