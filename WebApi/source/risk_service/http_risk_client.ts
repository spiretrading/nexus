import { RiskClient } from './risk_client';

/** Implements the RiskClient using HTTP requests. */
export class HttpRiskClient extends RiskClient {
  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
