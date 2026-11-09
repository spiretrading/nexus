import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';
import { RiskInventoryEntry, RiskPortfolioKey } from './risk_portfolio_types';

/** Publishes the current risk portfolio and subsequent inventory updates. */
export class RiskPortfolioPublisher implements
    Beam.Publisher<RiskInventoryEntry> {

  /** Constructs an empty portfolio publisher. */
  constructor() {
    this._entries = new Beam.Map<RiskPortfolioKey, Inventory>();
    this._publisher = new Beam.QueueWriterPublisher<RiskInventoryEntry>();
    this._exception = null;
  }

  /**
   * Updates the portfolio and notifies subscribers.
   * @param entry - The updated inventory and its portfolio key.
   * @param isRemoved - Whether to remove the entry from the portfolio.
   */
  public update(entry: RiskInventoryEntry, isRemoved: boolean): void {
    if(this._exception) {
      return;
    }
    if(isRemoved) {
      if(!this._entries.delete(entry.key)) {
        return;
      }
    } else {
      this._entries.set(entry.key, entry.value);
    }
    this._publisher.push(entry);
  }

  /**
   * Sends the current portfolio to a queue and subscribes it to later updates.
   * @param queue - The queue to receive the portfolio and its updates.
   */
  public monitor(queue: Beam.QueueWriter<RiskInventoryEntry>): void {
    if(this._exception) {
      queue.close(this._exception);
      return;
    }
    for(const [key, value] of this._entries) {
      queue.push(new RiskInventoryEntry(key, value));
    }
    this._publisher.monitor(queue);
  }

  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new Beam.PipeBrokenError();
    this._entries = new Beam.Map<RiskPortfolioKey, Inventory>();
    this._publisher.close(this._exception);
  }

  private _entries: Beam.Map<RiskPortfolioKey, Inventory>;
  private _publisher: Beam.QueueWriterPublisher<RiskInventoryEntry>;
  private _exception: Error;
}
