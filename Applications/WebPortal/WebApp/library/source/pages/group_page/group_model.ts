import * as Beam from 'beam';
import { AccountEntry } from '../account_directory_page';

/** Interface for a model representing a single group. */
export abstract class GroupModel {

  /** The group this model represents. */
  public abstract get group(): Beam.DirectoryEntry;

  /** Loads this model. */
  public abstract async load(): Promise<void>;
  
  /** Returns the accounts belonging to the group. */
  public abstract get accounts(): AccountEntry[];
}
