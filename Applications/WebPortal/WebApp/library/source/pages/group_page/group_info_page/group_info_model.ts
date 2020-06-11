import { AccountEntry } from '../..';

/** Interface for a model representing information of a group. */
export abstract class GroupInfoModel {

  /** Returns the accounts belonging to the group. */
  public abstract get group(): AccountEntry[];

  /** Loads the model. */
  public abstract async load(): Promise<void>;

  /** Used to compare accounts. */
  public accountComparator(accountA: AccountEntry,
      accountB: AccountEntry): number {
    return accountA.account.name.localeCompare(accountB.account.name);
  }
}
