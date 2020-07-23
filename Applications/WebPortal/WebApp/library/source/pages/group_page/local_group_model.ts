import * as Beam from 'beam';
import { GroupModel } from './group_model';

/** Implements an in-memory GroupModel. */
export class LocalGroupModel extends GroupModel {

  /** Constructs a LocalGroupModel. */
  constructor(group: Beam.DirectoryEntry) {
    super();
    this._isLoaded = false;
    this._group = group;
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get group(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._group;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _group: Beam.DirectoryEntry;
}
