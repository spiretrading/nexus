import * as Beam from 'beam';
import { CountryCode } from './country';
import { defaultMarketDatabase } from './default_market_database';
import { MarketCode, MarketDatabase } from './market';

/** Identifies a financial security. */
export class Security {

  /** Represents no Security. */
  public static readonly NONE = new Security('', MarketCode.NONE,
    CountryCode.NONE);

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Security {
    return new Security(value.symbol, MarketCode.fromJson(value.market),
      CountryCode.fromJson(value.country));
  }

  /** Parses a value from a string. */
  public static parse(source: string, marketDatabase?: MarketDatabase):
      Security {
    const seperator = source.lastIndexOf('.');
    if(seperator === -1) {
      return Security.NONE;
    }
    const symbol = source.substring(0, seperator);
    const marketSource = new MarketCode(source.substring(seperator + 1));
    const database = marketDatabase || defaultMarketDatabase;
    let market = database.fromDisplayName(marketSource.toString());
    if(market.code.equals(MarketCode.NONE)) {
      market = database.fromCode(marketSource);
      if(market.code.equals(MarketCode.NONE)) {
        return Security.NONE;
      }
    }
    return new Security(symbol, market.code, market.countryCode);
  }

  /**
   * Constructs a Security.
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

  /** Tests if two Securities are equal. */
  public equals(other: Security): boolean {
    return this.symbol == other.symbol && this.country.equals(other.country);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      symbol: this._symbol,
      market: this._market.toJson(),
      country: this._country.toJson()
    };
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return Beam.hashCombine(Beam.hash(this.symbol), Beam.hash(this.country));
  }

  public toString(marketDatabase?: MarketDatabase): string {
    if(this._market.equals(MarketCode.NONE) || this._symbol === '') {
      return this._symbol;
    }
    const database = marketDatabase || defaultMarketDatabase;
    const market = database.fromCode(this._market);
    if(market.code.equals(MarketCode.NONE)) {
      return `${this._symbol}.${this._market.toString()}`;
    }
    return `${this._symbol}.${market.displayName}`;
  }

  private _symbol: string;
  private _market: MarketCode;
  private _country: CountryCode;
}
