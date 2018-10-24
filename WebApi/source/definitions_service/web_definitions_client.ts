import * as Beam from 'beam';
import {CurrencyDatabase} from '..';
import {DefinitionsClient} from '.';

/** Implements the DefinitionsClient using web services. */
export class WebDefinitionsClient extends DefinitionsClient {

  /** Constructs a WebDefinitionsClient. */
  constructor() {
    super();
  }

  public get currencyDatabase(): CurrencyDatabase {
    return this._currencyDatabase;
  }

  public async open(): Promise<void> {
    try {
      const response = await Beam.post(
        '/api/definitions_service/load_currency_database', {});
      this._currencyDatabase = CurrencyDatabase.fromJson(response);
    } catch(e) {
      throw new Beam.ServiceError(e.statusText);
    }
  }

  public async close(): Promise<void> {
    return;
  }

  private _currencyDatabase: CurrencyDatabase;
}
