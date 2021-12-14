import * as Beam from 'beam';
import { ComplianceService } from '..';
import { AccountEntry } from '../account_directory_page';

/** Interface for a model representing a single group. */
export abstract class GroupModel {

  /** The group this model represents. */
  public abstract get group(): Beam.DirectoryEntry;

  /** Returns the accounts belonging to the group. */
  public abstract get accounts(): AccountEntry[];

  /** Returns the ComplianceService tied to the account. */
  public abstract get complianceService(): ComplianceService;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
