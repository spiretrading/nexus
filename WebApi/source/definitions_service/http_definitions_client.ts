import * as Beam from 'beam';
import { CountryDatabase, CurrencyDatabase, EntitlementDatabase,
  MarketDatabase } from '..';
import { DefinitionsClient } from './definitions_client';

/** Implements the DefinitionsClient using HTTP requests. */
export class HttpDefinitionsClient extends DefinitionsClient {
  public get entitlementDatabase(): EntitlementDatabase {
    return this._entitlementDatabase;
  }

  public get countryDatabase(): CountryDatabase {
    return this._countryDatabase;
  }

  public get currencyDatabase(): CurrencyDatabase {
    return this._currencyDatabase;
  }

  public get marketDatabase(): MarketDatabase {
    return this._marketDatabase;
  }

  public async open(): Promise<void> {
    const entitlementResponse = await Beam.post(
      '/api/administration_service/load_entitlements_database', {});
    this._entitlementDatabase = EntitlementDatabase.fromJson(
      entitlementResponse);
    const countryResponse = await Beam.post(
      '/api/definitions_service/load_country_database', {});
    this._countryDatabase = CountryDatabase.fromJson(countryResponse);
    const currencyResponse = await Beam.post(
      '/api/definitions_service/load_currency_database', {});
    this._currencyDatabase = CurrencyDatabase.fromJson(currencyResponse);
    const marketResponse = await Beam.post(
      '/api/definitions_service/load_market_database', {});
    this._marketDatabase = MarketDatabase.fromJson(marketResponse);
  }

  public async close(): Promise<void> {
    return;
  }

  private _entitlementDatabase: EntitlementDatabase;
  private _countryDatabase: CountryDatabase;
  private _currencyDatabase: CurrencyDatabase;
  private _marketDatabase: MarketDatabase;
}
