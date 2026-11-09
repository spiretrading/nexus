import * as Beam from 'beam';
import { Quantity } from '../definitions/quantity';
import { InventoryMessage } from './inventory_message';
import { InventoryUpdate } from './inventory_update';
import { RiskClient } from './risk_client';
import { RiskPortfolioPublisher } from './risk_portfolio_publisher';
import { RiskInventoryEntry, RiskPortfolioKey } from './risk_portfolio_types';
import { SubscribeRiskPortfolioUpdatesRequest } from
  './subscribe_risk_portfolio_updates_request';

/** Implements the RiskClient using the service protocol over WebSocket. */
export class HttpRiskClient extends RiskClient {

  /**
   * Constructs a client for the risk service.
   * @param serviceUrl - The risk service WebSocket endpoint.
   */
  constructor(serviceUrl: URL) {
    super();
    this._serviceClient = new Beam.ServiceProtocolClient(serviceUrl);
    this._publisher = new RiskPortfolioPublisher();
    this._isSubscribed = false;
    this._isInitialized = false;
    this._isClosed = false;
    this._pendingUpdates = [];
    this._opening = null;
    this._serviceClient.monitor(
      InventoryMessage, new Beam.CallbackQueueWriter<InventoryMessage>(
        this.onInventoryMessage, this.onBreak));
  }

  public getRiskPortfolioUpdatePublisher(): Beam.Publisher<RiskInventoryEntry> {
    if(!this._isSubscribed && !this._isClosed) {
      this._isSubscribed = true;
      this.subscribe();
    }
    return this._publisher;
  }

  public async open(): Promise<void> {
    if(this._isClosed) {
      throw new Beam.ServiceError('Risk client is closed.');
    }
    if(!this._opening) {
      this._opening = this._serviceClient.open();
    }
    await this._opening;
  }

  public async close(): Promise<void> {
    this._isClosed = true;
    this._pendingUpdates = [];
    this._publisher.close();
    this._serviceClient.close();
  }

  private async subscribe(): Promise<void> {
    try {
      await this.open();
      if(this._isClosed) {
        return;
      }
      const snapshot = await this._serviceClient.sendRequest(
        new SubscribeRiskPortfolioUpdatesRequest());
      if(this._isClosed) {
        return;
      }
      for(const entry of snapshot) {
        this._publisher.update(entry, entry.value.transactionCount === 0 &&
          entry.value.position.quantity.equals(Quantity.ZERO));
      }
      for(const update of this._pendingUpdates) {
        this.update(update);
      }
      this._pendingUpdates = [];
      this._isInitialized = true;
    } catch(error) {
      this.onBreak(error as Error);
    }
  }

  private update(update: InventoryUpdate): void {
    this._publisher.update(new RiskInventoryEntry(new RiskPortfolioKey(
      update.account, update.inventory.position.ticker), update.inventory),
      update.inventory.transactionCount === 0);
  }

  private onInventoryMessage = (message: InventoryMessage): void => {
    if(this._isClosed) {
      return;
    }
    if(!this._isInitialized) {
      this._pendingUpdates.push(...message.inventories);
      return;
    }
    for(const update of message.inventories) {
      this.update(update);
    }
  };

  private onBreak = (error: Error): void => {
    if(this._isClosed) {
      return;
    }
    this._isClosed = true;
    this._pendingUpdates = [];
    this._publisher.close(error);
    this._serviceClient.close();
  };

  private _serviceClient: Beam.ServiceProtocolClient;
  private _publisher: RiskPortfolioPublisher;
  private _isSubscribed: boolean;
  private _isInitialized: boolean;
  private _isClosed: boolean;
  private _pendingUpdates: InventoryUpdate[];
  private _opening: Promise<void>;
}
