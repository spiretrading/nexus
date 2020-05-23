import { AccountEntry } from '../..';

/** Interface for a model representing information of a group. */
export abstract class GroupInfoModel {

  /** Returns the accounts belonging to the group. */
  public abstract get group (): AccountEntry[];

  /** Loads the model. */
  public abstract async load(): Promise<void>;
}