import * as Beam from 'beam';
import { GroupSuggestionModel } from './group_suggestion_model';

/** Implements an GroupSuggestionModel in memory. */
export class LocalGroupSuggestionModel extends GroupSuggestionModel {

  /** Constructs an new model.
   * @param groups - A list of all available groups.
   */
  public constructor(groups: Beam.DirectoryEntry[]) {
    super();
    this._isLoaded = false;
    this._groups = groups.slice();
  }

  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async loadSuggestions(
      prefix: string): Promise<Beam.DirectoryEntry[]> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    const target = prefix.toLowerCase();
    const suggestions = this._groups.filter(
      group => group.name.toLowerCase().indexOf(target) === 0);
    return suggestions;
  }

  public addGroup(directory: Beam.DirectoryEntry): void {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    this._groups.push(directory);
  }

  private _isLoaded: boolean;
  private _groups: Beam.DirectoryEntry[];
}
