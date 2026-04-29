import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ComplianceService } from '..';
import { AccountEntry } from '../account_directory_page';
import { GroupProfitAndLossModel } from './group_profit_and_loss_page';

/** Interface for a model representing a single group. */
export abstract class GroupModel {

  /** The group this model represents. */
  public abstract get group(): Beam.DirectoryEntry;

  /** The group's base currency. */
  public abstract get currency(): Nexus.Currency;

  /** Returns the accounts belonging to the group. */
  public abstract get accounts(): AccountEntry[];

  /** Returns the ComplianceService tied to the account. */
  public abstract get complianceService(): ComplianceService;

  /** Returns the GroupProfitAndLossModel. */
  public abstract get profitAndLossModel(): GroupProfitAndLossModel;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
