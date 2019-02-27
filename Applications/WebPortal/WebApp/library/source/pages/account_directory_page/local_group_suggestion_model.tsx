import * as Beam from 'beam';
import GroupSuggestionModel from './group_suggestion_model';

/** Interface for model  */
export class LocalGroupSuggestionModel extends GroupSuggestionModel {

  /** Constructs an new model.
   * @param groups - A set of groups.
   */
  
  // Loads this model.
  public abstract async load(): Promise<void>;

  // Returns all(?) the accounts with the given prefix.
  public abstract async loadSuggestions(prefix: string):
    Promise<Beam.DirectoryEntry[]>;
}
