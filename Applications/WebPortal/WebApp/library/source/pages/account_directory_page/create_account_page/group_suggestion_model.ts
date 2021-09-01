import * as Beam from 'beam';

/** Interface for model  */
export abstract class GroupSuggestionModel {

  // Loads this model.
  public abstract load(): Promise<void>;

  /** Returns all the groups with the given prefix.
   * If the filter is a empty group is returned.
   * @param prefix - The string that is the current prefix.
   */
  public abstract loadSuggestions(prefix: string):
    Promise<Beam.DirectoryEntry[]>;

  /** Add the group to the list of groups loaded.
   * @param directory - The group to add.
   */
  public abstract addGroup(directory: Beam.DirectoryEntry): void;
}
