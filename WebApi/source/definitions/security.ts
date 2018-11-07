import { CountryCode, MarketCode } from '.';

/** Identifies a financial security. */
export class Security {

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

  private _symbol: string;
  private _market: MarketCode;
  private _country: CountryCode;
}
