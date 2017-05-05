import CurrencyPair from '../currency-pair';
import HashMap from 'hashmap';
import ExchangeRate from '../exchange-rate';

class ExchangeRateTable {
  constructor() {
    this.exchangeRates = new HashMap();
  }

  add(exchangeRate) {
    let baseCurrencyId = exchangeRate.currencyPair.base.toNumber();
    let counterCurrencyId = exchangeRate.currencyPair.counter.toNumber();
    let counterExchangeRates;
    if (this.exchangeRates.has.apply(this.exchangeRates, [baseCurrencyId])) {
      counterExchangeRates = this.exchangeRates.get.apply(this.exchangeRates, [baseCurrencyId]);
    } else {
      counterExchangeRates = new HashMap();
      this.exchangeRates.set.apply(this.exchangeRates, [baseCurrencyId, counterExchangeRates]);
    }
    counterExchangeRates.set.apply(counterExchangeRates, [counterCurrencyId, exchangeRate]);
  }

  find(currencyPair) {
    let baseCurrencyId = currencyPair.base.toNumber();
    let counterCurrencyId = currencyPair.counter.toNumber();
    let rate = findRate.apply(this, [baseCurrencyId, counterCurrencyId]);
    if (rate != null) {
      return rate;
    }

    baseCurrencyId = currencyPair.counter.toNumber();
    counterCurrencyId = currencyPair.base.toNumber();
    rate = findRate.apply(this, [baseCurrencyId, counterCurrencyId]);
    if (rate != null) {
      return ExchangeRate.invert(rate);
    }

    return null;

    function findRate(baseCurrencyId, counterCurrencyId) {
      if (this.exchangeRates.has.apply(this.exchangeRates, [baseCurrencyId])) {
        let counterExchangeRates = this.exchangeRates.get.apply(this.exchangeRates, [baseCurrencyId]);
        return counterExchangeRates.get.apply(counterExchangeRates, [counterCurrencyId]);
      }
      return null;
    }
  }

  convert(amount, baseCurrencyId, counterCurrencyId) {
    let currencyPair = new CurrencyPair(baseCurrencyId, counterCurrencyId);
    let rate = this.find.apply(this, [currencyPair]);
    return ExchangeRate.convert(amount, rate);
  }
}

export default ExchangeRateTable;
