import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';
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
    this._clientHandler = new Beam.ServiceProtocolClientHandler(
      serviceUrl, client => this.recover(client));
    this._publisher = new RiskPortfolioPublisher();
    this._isSubscribed = false;
    this._isInitialized = false;
    this._isClosed = false;
    this._pendingUpdates = [];
    this._subscriptionClient = null;
    this._clientHandler.monitor(
      InventoryMessage, new Beam.CallbackQueueWriter<InventoryMessage>(
        this.onInventoryMessage, this.onBreak));
  }

  public getRiskPortfolioUpdatePublisher(): Beam.SnapshotPublisher<
      RiskInventoryEntry, Beam.Map<RiskPortfolioKey, Inventory>> {
    if(!this._isSubscribed && !this._isClosed) {
      this._isSubscribed = true;
      this.subscribe();
    }
    return this._publisher;
  }

  /**
   * Connects to the service, retrying connection failures until closed.
   * Waits for portfolio recovery when reconnecting.
   */
  public async open(): Promise<void> {
    await this._clientHandler.getClient();
  }

  public async close(): Promise<void> {
    this.onBreak(new Beam.PipeBrokenError());
  }

  private async subscribe(): Promise<void> {
    try {
      const client = await this._clientHandler.getClient();
      if(!this._isClosed && client !== this._subscriptionClient) {
        await this.initialize(client);
      }
    } catch(error) {
      if(!(error instanceof Beam.PipeBrokenError)) {
        this.onBreak(error as Error);
      }
    }
  }

  private async initialize(client: Beam.ServiceProtocolClient): Promise<void> {
    this._subscriptionClient = client;
    this._isInitialized = false;
    this._pendingUpdates = [];
    const snapshot = await client.sendRequest(
      new SubscribeRiskPortfolioUpdatesRequest());
    if(this._isClosed || client !== this._subscriptionClient) {
      return;
    }
    const entries = new Beam.Map<RiskPortfolioKey, Inventory>();
    for(const entry of snapshot) {
      if(entry.value.transactionCount !== 0 ||
          !entry.value.position.quantity.equals(Quantity.ZERO)) {
        entries.set(entry.key, entry.value);
      }
    }
    for(const [key, inventory] of this._publisher.getSnapshot()) {
      if(!entries.has(key)) {
        this._publisher.update(new RiskInventoryEntry(key,
          new Inventory(inventory.position.ticker,
            inventory.position.currency)), true);
      }
    }
    for(const [key, inventory] of entries) {
      this._publisher.update(new RiskInventoryEntry(key, inventory), false);
    }
    for(const update of this._pendingUpdates) {
      this.update(update);
    }
    this._pendingUpdates = [];
    this._isInitialized = true;
  }

  private async recover(client: Beam.ServiceProtocolClient): Promise<void> {
    if(this._isSubscribed && !this._isClosed) {
      await this.initialize(client);
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
    this._clientHandler.close(error);
  };

  private _clientHandler: Beam.ServiceProtocolClientHandler;
  private _publisher: RiskPortfolioPublisher;
  private _isSubscribed: boolean;
  private _isInitialized: boolean;
  private _isClosed: boolean;
  private _pendingUpdates: InventoryUpdate[];
  private _subscriptionClient: Beam.ServiceProtocolClient;
}
