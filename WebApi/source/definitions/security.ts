import { CountryCode } from './country';
import { MarketCode } from './market';

/** Identifies a financial security. */
export class Security {

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Security {
    return new Security(value.symbol, MarketCode.fromJson(value.market),
      CountryCode.fromJson(value.country));
  }

  /** Constructs a Security.
   * @param symbol - The security's ticker symbol.
   * @param market - The security's market code.
   * @param country - The security's country of origin.
   */
  public constructor(symbol: string, market: MarketCode, country: CountryCode) {
    this._symbol = symbol;
    this._market = market;
    this._country = country;
  }

  /** Returns the ticker symbol. */
  public get symbol(): string {
    return this._symbol;
  }

  /** Returns the market code. */
  public get market(): MarketCode {
    return this._market;
  }

  /** Returns the country of origin. */
  public get country(): CountryCode {
    return this._country;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      symbol: this._symbol,
      market: this._market.toJson(),
      country: this._country.toJson()
    };
  }

  private _symbol: string;
  private _market: MarketCode;
  private _country: CountryCode;
}
