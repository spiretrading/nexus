import * as Beam from 'beam';
import {GroupSuggestionModel} from './group_suggestion_model';

/** Interface for model  */
export class LocalGroupSuggestionModel extends GroupSuggestionModel {

  /** Constructs an new model.
   * @param groups - A set of groups.
   */
  public constructor(groups: Beam.Set<Beam.DirectoryEntry>) {
    console.log('Made a model');
    super();
    this._groups = groups.clone();
  }

  public isLoaded(): boolean {
    return this._isLoaded;
  }
  // Loads this model.
  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  // Returns all the accounts with the given prefix.
  public async loadSuggestions(
      prefix: string): Promise<Beam.Set<Beam.DirectoryEntry>> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    console.log('Loading a thing');
    return new Promise<Beam.Set<Beam.DirectoryEntry>> ((resolve) => {
      setTimeout(() => {
        const set = new Beam.Set<Beam.DirectoryEntry>();
        if(prefix) {
          for(const group of this._groups) {
            if(group.name.indexOf(prefix) === 0) {
              set.add(group);
            }
          }
        }
        resolve(set);}, 100);
      });
  }

  private _isLoaded: boolean;
  private _groups: Beam.Set<Beam.DirectoryEntry>;
}
