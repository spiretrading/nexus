import {MarketCode} from './market';
import {CountryCode} from './country';

class Security {
  public constructor(symbol: string, market: MarketCode, country: CountryCode) {
    this._symbol = symbol;
    this._market = market;
    this._country = country;
  }

  public get symbol(): string {
    return this._symbol;
  }

  public get market(): MarketCode {
    return this._market;
  }

  public get country(): CountryCode {
    return this._country;
  }

  private _symbol: string;
  private _market: MarketCode;
  private _country: CountryCode;
}

export {Security};
