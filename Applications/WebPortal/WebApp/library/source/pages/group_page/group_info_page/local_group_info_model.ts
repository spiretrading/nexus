import { AccountEntry } from '../../..';
import { GroupInfoModel } from './group_info_model';

/** Implements an in-memory GroupInfoModel. */
export class LocalGroupInfoModel extends GroupInfoModel {

  /** Constructs a LocalGroupInfoModel. */
  constructor(group: AccountEntry[]) {
    super();
    this._isLoaded = false;
    this._group = group.slice();
  }

  public get group(): AccountEntry[] {
    return this._group;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _group: AccountEntry[];
}
