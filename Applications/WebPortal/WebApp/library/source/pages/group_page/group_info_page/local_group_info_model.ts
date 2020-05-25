import { AccountEntry } from '../../..';
import { GroupInfoModel } from './group_info_model';

/** Implements an in-memory GroupInfoModel. */
export class LocalGroupInfoModel extends GroupInfoModel {

  /** Constructs a LocalGroupInfoModel. 
   *@param group: The list of account entries that form the group. 
   */
  constructor(group: AccountEntry[]) {
    super();
    this._isLoaded = false;
    this._group = group.slice();
  }

  public get group(): AccountEntry[] {
    return this._group.slice();
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _group: AccountEntry[];
}
