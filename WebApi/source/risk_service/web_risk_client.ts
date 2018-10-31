import {RiskClient} from '.';

/** Implements the RiskClient using web services. */
export class WebRiskClient extends RiskClient {

  /** Constructs a WebRiskClient. */
  constructor() {
    super();
  }

  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
