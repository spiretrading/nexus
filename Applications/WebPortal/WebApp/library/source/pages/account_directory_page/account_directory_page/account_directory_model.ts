import * as Beam from 'beam';
import { CreateAccountModel, GroupSuggestionModel } from
  '../create_account_page';
import { AccountEntry } from './account_entry';

/** Interface for a model representing a directory of accounts. */
export abstract class AccountDirectoryModel {

  /** Returns a list of all groups. */
  public abstract get groups(): Beam.DirectoryEntry[];

  /** Returns a CreateAccountModel. */
  public abstract get createAccountModel(): CreateAccountModel;

  /** Return a GroupSuggestionModel. */
  public abstract get groupSuggestionModel(): GroupSuggestionModel;

  /**
   * Creates a new group.
   * @param name The name of the group.
   * @return The newly created group's DirectoryEntry.
   */
  public abstract createGroup(name: string): Promise<Beam.DirectoryEntry>;

  /** Returns the accounts that belong to a particular group. */
  public abstract loadAccounts(group: Beam.DirectoryEntry):
    Promise<AccountEntry[]>;

  /**
   * Returns all the accounts where the filter is a prefix to the name.
   * If the filter is a empty string a empty map is returned.
   * @param filter - The string that is the current filter.
   */
  public abstract loadFilteredAccounts(filter: string):
    Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
