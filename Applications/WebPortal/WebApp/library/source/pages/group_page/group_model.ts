import * as Beam from 'beam';

/** Interface for a model representing a single group. */
export abstract class GroupModel {

  /** The group this model represents. */
  public abstract get group(): Beam.DirectoryEntry;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
}
