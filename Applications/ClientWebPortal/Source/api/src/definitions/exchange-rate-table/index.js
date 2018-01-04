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
    if (this.exchangeRates.has(baseCurrencyId)) {
      counterExchangeRates = this.exchangeRates.get(baseCurrencyId);
    } else {
      counterExchangeRates = new HashMap();
      this.exchangeRates.set(baseCurrencyId, counterExchangeRates);
    }
    counterExchangeRates.set(counterCurrencyId, exchangeRate);
  }

  find(currencyPair) {
    let baseCurrencyId = currencyPair.base.toNumber();
    let counterCurrencyId = currencyPair.counter.toNumber();
    let rate = findRate.call(this, baseCurrencyId, counterCurrencyId);
    if (rate != null) {
      return rate;
    }

    baseCurrencyId = currencyPair.counter.toNumber();
    counterCurrencyId = currencyPair.base.toNumber();
    rate = findRate.call(this, baseCurrencyId, counterCurrencyId);
    if (rate != null) {
      return ExchangeRate.invert(rate);
    }

    return null;

    function findRate(baseCurrencyId, counterCurrencyId) {
      if (this.exchangeRates.has(baseCurrencyId)) {
        let counterExchangeRates = this.exchangeRates.get(baseCurrencyId);
        return counterExchangeRates.get(counterCurrencyId);
      }
      return null;
    }
  }

  convert(amount, baseCurrencyId, counterCurrencyId) {
    if (baseCurrencyId.equals(counterCurrencyId)) {
      let rate = ExchangeRate.getIdentical();
      return ExchangeRate.convert(amount, rate);
    } else {
      let currencyPair = new CurrencyPair(baseCurrencyId, counterCurrencyId);
      let rate = this.find(currencyPair);
      return ExchangeRate.convert(amount, rate);
    }
  }
}

export default ExchangeRateTable;
