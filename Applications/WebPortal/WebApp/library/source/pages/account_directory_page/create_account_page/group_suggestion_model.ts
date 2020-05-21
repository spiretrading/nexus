import * as Beam from 'beam';

/** Interface for model  */
export abstract class GroupSuggestionModel {

  // Loads this model.
  public abstract async load(): Promise<void>;

  /** Returns all the groups with the given prefix.
   * If the filter is a empty group is returned.
   * @param prefix - The string that is the current prefix.
   */
  public abstract async loadSuggestions(prefix: string):
    Promise<Beam.DirectoryEntry[]>;
}
