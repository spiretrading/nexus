import * as Beam from 'beam';
import { QueryModel } from '../../models';

/** Looks up accounts and groups available to the current user. */
export abstract class AccountGroupQueryModel extends
    QueryModel<Beam.DirectoryEntry> {

  /** Resolves a case-insensitive account or group name.
   * @return The entry, or null if the name is unknown or ambiguous.
   */
  public async parse(query: string): Promise<Beam.DirectoryEntry> {
    const name = query.trim().toLocaleLowerCase();
    const entries = await this.submit(query);
    let result: Beam.DirectoryEntry = null;
    for(const entry of entries) {
      if(entry.name.trim().toLocaleLowerCase() === name) {
        if(result && !result.equals(entry)) {
          return null;
        }
        result = entry;
      }
    }
    return result;
  }
}
