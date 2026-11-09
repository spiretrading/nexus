import * as Beam from 'beam';
import { RiskInventoryEntry } from './risk_portfolio_types';

/** Client used to access risk services. */
export abstract class RiskClient {

  /** Returns the publisher of the current portfolio and subsequent updates. */
  public abstract getRiskPortfolioUpdatePublisher():
      Beam.Publisher<RiskInventoryEntry>;

  /**
   * Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
