import * as Beam from 'beam';
import {CurrencyDatabase} from '..';

/** Client used to access definitions services. */
export abstract class DefinitionsClient {

  /** Returns the currency database. */
  public abstract get currencyDatabase(): CurrencyDatabase;

  /** Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract async open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract async close(): Promise<void>;
}
