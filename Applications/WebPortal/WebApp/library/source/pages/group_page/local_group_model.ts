import * as Beam from 'beam';
import { ComplianceModel, ComplianceService, LocalComplianceService } from
  '..';
import { AccountEntry } from '../account_directory_page';
import { GroupModel } from './group_model';

/** Implements an in-memory GroupModel. */
export class LocalGroupModel extends GroupModel {

  /**
   * Constructs a LocalGroupModel.
   * @param group The group represented.
   * @param accounts The list of accounts belonging to this group.
   * @param complianceModel The group's compliance model.
   */
  constructor(group: Beam.DirectoryEntry, accounts: AccountEntry[],
      complianceModel: ComplianceModel) {
    super();
    this._isLoaded = false;
    this._group = group;
    this._accounts = accounts.slice();
    this._complianceService = new LocalComplianceService(complianceModel);
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get group(): Beam.DirectoryEntry {
    this.ensureLoaded();
    return this._group;
  }

  public get accounts(): AccountEntry[] {
    this.ensureLoaded();
    return this._accounts.slice();
  }

  public get complianceService(): ComplianceService {
    this.ensureLoaded();
    return this._complianceService;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
  }

  private _isLoaded: boolean;
  private _group: Beam.DirectoryEntry;
  private _accounts: AccountEntry[];
  private _complianceService: ComplianceService;
}
