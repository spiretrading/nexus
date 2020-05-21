import { AccountEntry } from '../..';

export abstract class GroupInfoModel {

  /** Returns the accounts belonging to the group. */
  public abstract get groupMembers (): AccountEntry[]; 
}