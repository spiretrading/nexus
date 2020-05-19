import * as Beam from 'beam';
import { AccountEntry } from './account_entry';

/** Interface for a model representing a directory of accounts. */
export abstract class AccountDirectoryModel {

  /** Returns a list of all groups. */
  public abstract get groups(): Beam.Set<Beam.DirectoryEntry>;

  /**
   * Creates a new group.
   * @param name The name of the group.
   * @return The newly created group's DirectoryEntry.
   */
  public abstract async createGroup(name: string): Promise<Beam.DirectoryEntry>;

  /** Returns the accounts that belong to a particular group. */
  public abstract async loadAccounts(
    group: Beam.DirectoryEntry): Promise<AccountEntry[]>;

  /** Returns all the accounts where the filter is a prefix to the name.
   * If the filter is a empty string a empty map is returned.
   * @param filter - The string that is the current filter.
   */
  public abstract async loadFilteredAccounts(
    filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
