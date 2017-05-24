import CountryCode from '../country-code';
import Security from '../security';

class SecuritySet {
  constructor(symbol, market, country) {
    // TODO: implement set operations using a set collection
  }
}

const SYMBOL_WILD_CARD = '*';
const MARKET_CODE_WILD_CARD = '*';

SecuritySet.SYMBOL_WILD_CARD = SYMBOL_WILD_CARD;
SecuritySet.MARKET_CODE_WILD_CARD = MARKET_CODE_WILD_CARD;

SecuritySet.toWildCardString = security => {
  if (security.symbol == SecuritySet.SYMBOL_WILD_CARD &&
      security.market == SecuritySet.MARKET_CODE_WILD_CARD &&
      security.country == CountryCode.NONE) {
    return '*';
  }
};

SecuritySet.parseWildCardSecurity = source => {
  if (source === '*' || source === '*.*' || source === '*.*.*') {
    return Security.getWildCardSecurity();
  }
};

export default SecuritySet;
