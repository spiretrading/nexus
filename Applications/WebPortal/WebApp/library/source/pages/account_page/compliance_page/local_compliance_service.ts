import * as Nexus from 'nexus';
import { QueryModel } from '../../../models';
import { ComplianceModel } from './compliance_model';
import { ComplianceService } from './compliance_service';

/** Implements an in-memory ComplianceService. */
export class LocalComplianceService extends ComplianceService {

  /**
   * Constructs a LocalComplianceService.
   * @param model The model to return on load.
   * @param tickers The ticker lookup provider.
   */
  constructor(model: ComplianceModel, tickers: QueryModel<Nexus.Ticker>) {
    super(tickers);
    this.model = model.clone();
  }

  public async load(): Promise<ComplianceModel> {
    return this.model.clone();
  }

  public async submit(model: ComplianceModel): Promise<ComplianceModel> {
    return await this.load();
  }

  private model: ComplianceModel;
}
