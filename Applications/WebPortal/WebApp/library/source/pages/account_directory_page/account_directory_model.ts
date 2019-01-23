import * as Beam from 'beam';
import { AccountEntry } from './account_entry';

/** Interface for a model representing an directory of accounts. */
export abstract class AccountDirectoryModel {

  /** Returns a list of all groups. */
  public abstract get groups(): Beam.Set<Beam.DirectoryEntry>;

  /** Returns the accounts that belong to a particular group. */
  public abstract async loadAccounts(
    group: Beam.DirectoryEntry): Promise<AccountEntry[]>;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
