import * as Nexus from 'nexus';
import { QueryModel, ScopeQueryModel } from '../../../models';
import { ComplianceModel } from './compliance_model';

/** Interface for the services needed by the CompliancePage. */
export abstract class ComplianceService {

  /** Constructs the service with its ticker lookup provider. */
  constructor(tickers: QueryModel<Nexus.Ticker>) {
    this.tickers = tickers;
    this.scopes = new ScopeQueryModel(tickers);
  }

  /** Returns the ticker lookup provider. */
  public get tickerQueryModel(): QueryModel<Nexus.Ticker> {
    return this.tickers;
  }

  /** Returns the scope lookup provider. */
  public get scopeQueryModel(): QueryModel<Nexus.Scope> {
    return this.scopes;
  }

  /** Loads this model. */
  public abstract load(): Promise<ComplianceModel>;

  /**
   * Submits a request to commit all changes made to a ComplianceModel.
   * @param model The ComplianceModel to commit.
   * @return The updated ComplianceModel after the changes have been committed.
   */
  public abstract submit(model: ComplianceModel): Promise<ComplianceModel>;

  private tickers: QueryModel<Nexus.Ticker>;
  private scopes: QueryModel<Nexus.Scope>;
}
