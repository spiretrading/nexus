import CurrencyId from '../currency-id';

class CurrencyPair {
  /*
    param base: CurrencyId, counter: CurrencyId
  */
  constructor(base, counter) {
    this.base = base;
    this.counter = counter;
  }

  clone() {
    let baseClone = this.base.clone.apply(this.base);
    let counterClone = this.counter.clone.apply(this.counter);
    return new CurrencyPair(baseClone, counterClone);
  }
}

CurrencyPair.parse = (symbol, currencyDatabase) => {
  let slashIndex = symbol.indexOf('/');
  if (slashIndex < 0) {
    throw new Error('Missing separator "/"');
  }

  let baseCode = symbol.substring(0, slashIndex);
  let counterCode = symbol.substring(slashIndex + 1);

  let baseCurrency = currencyDatabase.fromCode(baseCode);
  if (baseCurrency == null) {
    throw new Error('Invalid base currency code');
  }
  let baseCurrencyId = new CurrencyId(baseCurrency.id.toNumber());

  let counterCurrency = currencyDatabase.fromCode(counterCode);
  if (counterCurrency == null) {
    throw new Error('Invalid counter currency code');
  }
  let counterCurrencyId = new CurrencyId(counterCurrency.id.toNumber());

  return new CurrencyPair(baseCurrencyId, counterCurrencyId);
};

CurrencyPair.invert = pair => {
  return new CurrencyPair(pair.counter, pair.base);
};

export default CurrencyPair;
