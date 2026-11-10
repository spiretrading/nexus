import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';
import { RiskInventoryEntry, RiskPortfolioKey } from './risk_portfolio_types';

/** Client used to access risk services. */
export abstract class RiskClient {

  /** Returns the publisher of the current portfolio and subsequent updates. */
  public abstract getRiskPortfolioUpdatePublisher():
      Beam.SnapshotPublisher<
        RiskInventoryEntry, Beam.Map<RiskPortfolioKey, Inventory>>;

  /**
   * Connects to the service.
   * @throws Error If the client is closed or a terminal connection failure
   * occurs.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
