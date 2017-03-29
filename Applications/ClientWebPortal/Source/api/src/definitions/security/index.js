import MarketCode from '../market/code';
import CountryCode from '../country/code';

class Security {
  constructor(country, market, symbol) {
    this.country = country;
    this.market = market;
    this.symbol = symbol;
  }
}

Security.fromData = (data) => {
  let country = new CountryCode(data.country);
  let market = new MarketCode(data.market);
  return new Security(country, market, data.symbol);
};

export default Security;
