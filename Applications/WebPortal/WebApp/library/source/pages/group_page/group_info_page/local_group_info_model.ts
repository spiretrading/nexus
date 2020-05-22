import { AccountEntry } from '../../..';
import { GroupInfoModel } from './group_info_model';

/** Implements an in-memory GroupInfoModel. */
export class LocalGroupInfoModel extends GroupInfoModel {

  /** Constructs a LocalGroupInfoModel. */
  constructor(group: AccountEntry[]) {
    super();
    this._isLoaded = false;
    this._groupMembers = group.slice();
  }

  public get groupMembers(): AccountEntry[] {
    return this._groupMembers;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _groupMembers: AccountEntry[];
}
