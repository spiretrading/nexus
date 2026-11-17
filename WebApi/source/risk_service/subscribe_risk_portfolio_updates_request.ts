import * as Beam from 'beam';
import { RiskInventoryEntry } from './risk_portfolio_types';

/** Subscribes to the portfolio updates accessible to the current account. */
export class SubscribeRiskPortfolioUpdatesRequest implements
    Beam.ServiceRequest<RiskInventoryEntry[]> {
  public readonly service =
    'Nexus.RiskService.SubscribeRiskPortfolioUpdatesService';

  public toJson(): any {
    return {};
  }

  public parseResponse(value: any): RiskInventoryEntry[] {
    return Beam.arrayFromJson(RiskInventoryEntry, value);
  }
}
