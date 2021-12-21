import * as Beam from 'beam';
import { CountryCode, CountryDatabase } from './country';
import { defaultCountryDatabase } from './default_country_database';
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

/**
 * Parses a string that potentially represents a wild card Security.
 * @param source The string to parse.
 * @param marketDatabase The database of markets to reference.
 * @param countryDatabase The database of countries to reference.
 * @return A Security potentially representing a wild card.
 */
export function parseWildCardSecurity(
    source: string, marketDatabase?: MarketDatabase,
    countryDatabase?: CountryDatabase): Security {
  marketDatabase = marketDatabase || defaultMarketDatabase;
  countryDatabase = countryDatabase || defaultCountryDatabase;
  if(source === '*' || source === '*.*' || source === '*.*.*') {
    return new Security('*', new MarketCode('*'), CountryCode.NONE);
  }
  const seperator = source.lastIndexOf('.');
  if(seperator === -1) {
    return Security.NONE;
  }
  const header = source.substring(0, seperator);
  const trailer = source.substring(seperator + 1);
  if(header === '*') {
    const market = MarketDatabase.Entry.parse(trailer, marketDatabase);
    if(!market.code.equals(MarketCode.NONE)) {
      return new Security(header, market.code, market.countryCode);
    }
  }
  const prefixSecurity =
    parseWildCardSecurity(header, marketDatabase, countryDatabase);
  if(!prefixSecurity.equals(Security.NONE)) {
    if(trailer.length == 2) {
      const code = countryDatabase.fromLetterCode(trailer);
      if(!code.code.equals(CountryCode.NONE)) {
        return new Security(
          prefixSecurity.symbol, prefixSecurity.market, code.code);
      } else {
        return Security.NONE;
      }
    } else if(trailer === '*') {
      return new Security(
        prefixSecurity.symbol, prefixSecurity.market, CountryCode.NONE);
    } else {
      return Security.NONE;
    }
  }
  let market = null;
  let country = null;
  if(trailer === '*') {
    [market, country] = [new MarketCode('*'), CountryCode.NONE];
  } else {
    const marketEntry = MarketDatabase.Entry.parse(trailer, marketDatabase);
    if(marketEntry.code.equals(MarketCode.NONE)) {
      return Security.NONE;
    }
    [market, country] = [marketEntry.code, marketEntry.countryCode];
  }
  return new Security(header, market, country);
}


/**
 * Returns the string representation of a Security, including wild-cards.
 * @param security The Security to represent.
 * @param marketDatabase The MarketDatabase used to represent the MarketCode.
 * @param countryDatabase The CountryDatabase used to represent the CountryCode.
 * @return The string representation of the security.
 */
export function toWildCardString(security: Security,
    marketDatabase?: MarketDatabase, countryDatabase?: CountryDatabase) {
  marketDatabase = marketDatabase || defaultMarketDatabase;
  countryDatabase = countryDatabase || defaultCountryDatabase;
  if(security.symbol === '*' && security.market.toString() === '*' &&
      security.country.equals(CountryCode.NONE)) {
    return '*';
  } else if(security.equals(Security.NONE)) {
    return '';
  }
  const suffix = (() => {
    if(security.market.toString() === '*') {
      if(!security.country.equals(CountryCode.NONE)) {
        const countryEntry = countryDatabase.fromCode(security.country);
        return countryEntry.twoLetterCode;
      }
      return '*';
    }
    const market = marketDatabase.fromCode(security.market);
    return market.displayName;
  })();
  return `${security.symbol}.${suffix}`;
}