import { AccountEntry } from '../..';

/** Interface for a model representing information of a group. */
export abstract class GroupInfoModel {

  /** Returns the accounts belonging to the group. */
  public abstract get groupMembers (): AccountEntry[];

  /** Loads the model. */
  public abstract async load(): Promise<void>;
}