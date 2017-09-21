import MarketCode from '../market-code';
import CountryCode from '../country-code';
import SecuritySet from '../security-set';

class Security {
  constructor(country, market, symbol) {
    this.country = country;
    this.market = market;
    this.symbol = symbol;
    this.constructor = {
      name: 'Security'
    };
    this.toString = this.toString.bind(this);
  }

  toData() {
    if (this.country == CountryCode.NONE && this.market == SecuritySet.MARKET_CODE_WILD_CARD && this.symbol == SecuritySet.SYMBOL_WILD_CARD) {
      return {
        country: this.country,
        market: this.market,
        symbol: this.symbol
      };
    } else {
      return {
        country: this.country.toNumber(),
        market: this.market.toData(),
        symbol: this.symbol
      };
    }
  }

  toString(marketDatabase) {
    if (this.market.isEmpty() || this.symbol == '') {
      return this.symbol;
    }
    let marketCode = this.market.toData();
    let market = marketDatabase.fromMarketCode(marketCode);
    if (market == null || market.marketCode == '') {
      return this.symbol + '.' + marketCode;
    } else {
      return this.symbol + '.' + market.displayName;
    }
  }

  clone() {
    return new Security(this.country, this.market, this.symbol);
  }
}

Security.fromData = data => {
  let country = new CountryCode(data.country);
  let market = new MarketCode(data.market);
  return new Security(country, market, data.symbol);
};

Security.getWildCard = () => {
  let country = new CountryCode(CountryCode.NONE);
  let market = new MarketCode(SecuritySet.MARKET_CODE_WILD_CARD);
  return new Security(country, market, SecuritySet.SYMBOL_WILD_CARD);
};

Security.isWildCard = securityLabel => {
  if (securityLabel === '*' || securityLabel === '*.*' || securityLabel === '*.*.*') {
    return true;
  } else {
    return false;
  }
};

export default Security;
