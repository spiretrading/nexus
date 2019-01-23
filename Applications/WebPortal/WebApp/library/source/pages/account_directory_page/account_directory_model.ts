import * as Beam from 'beam';
import { AccountEntry } from './account_entry';

/** Interface for a model representing an directory of accounts. */
export abstract class AccountDirectoryModel {

  /** Loads this model. */
  public abstract async load(): Promise<void>;

  /** Returns a list of all groups. */
  public abstract get groups(): Beam.DirectoryEntry[];

  /** Returns the accounts that belong to a particular group. */
  public abstract async loadAccounts(group: Beam.DirectoryEntry
    ): Promise<AccountEntry[]>;

}
