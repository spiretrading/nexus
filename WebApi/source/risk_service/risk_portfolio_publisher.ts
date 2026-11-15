import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';
import { RiskInventoryEntry, RiskPortfolioKey } from './risk_portfolio_types';

/** Publishes the current risk portfolio and subsequent inventory updates. */
export class RiskPortfolioPublisher implements Beam.SnapshotPublisher<
    RiskInventoryEntry, Beam.Map<RiskPortfolioKey, Inventory>> {

  /** Constructs an empty portfolio publisher. */
  constructor() {
    this._publisher = new Beam.TablePublisher<RiskPortfolioKey, Inventory>();
    this._isClosed = false;
  }

  /**
   * Updates the portfolio and notifies subscribers.
   * @param entry - The updated inventory and its portfolio key.
   * @param isRemoved - Whether to remove the entry from the portfolio.
   */
  public update(entry: RiskInventoryEntry, isRemoved: boolean): void {
    if(this._isClosed) {
      return;
    }
    if(isRemoved) {
      this._publisher.erase(entry.key, entry.value);
    } else {
      this._publisher.push(entry.key, entry.value);
    }
  }

  public getSnapshot(): Beam.Map<RiskPortfolioKey, Inventory> {
    return this._publisher.getSnapshot();
  }

  public monitor(queue: Beam.QueueWriter<RiskInventoryEntry>): void;
  public monitor(queue: Beam.QueueWriter<RiskInventoryEntry>,
    snapshot: { value: Beam.Map<RiskPortfolioKey, Inventory> }): void;
  public monitor(queue: Beam.QueueWriter<RiskInventoryEntry>,
      snapshot?: { value: Beam.Map<RiskPortfolioKey, Inventory> }): void {
    const writer = new Beam.CallbackQueueWriter<
      Beam.KeyValuePair<RiskPortfolioKey, Inventory>>(
        entry => queue.push(new RiskInventoryEntry(entry.key, entry.value)),
        error => queue.close(error));
    if(snapshot) {
      this._publisher.monitor(writer, snapshot);
    } else {
      this._publisher.monitor(writer);
    }
  }

  /**
   * Closes monitored queues and retains the current portfolio.
   * @param error - The error used to close the queues.
   */
  public close(error?: Error): void {
    if(this._isClosed) {
      return;
    }
    this._isClosed = true;
    this._publisher.close(error);
  }

  private _publisher: Beam.TablePublisher<RiskPortfolioKey, Inventory>;
  private _isClosed: boolean;
}
