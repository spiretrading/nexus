import * as Beam from 'beam';

/** Interface for model  */
export abstract class GroupSuggestionModel {

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return true;
  }

  // Loads this model.
  public abstract async load(): Promise<void>;

  // Returns all(?) the accounts with the given prefix.
  public abstract async loadSuggestions(prefix: string):
    Promise<Beam.DirectoryEntry[]>;
}
