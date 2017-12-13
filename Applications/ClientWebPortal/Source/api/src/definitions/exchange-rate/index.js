import CurrencyPair from '../currency-pair';
import Fraction from 'fraction.js';

class ExchangeRate {
  constructor(currencyPair, rate) {
    this.currencyPair = currencyPair;
    this.rate = new Fraction(rate);
  }

  clone() {
    let currencyPairClone = this.currencyPair.clone();
    return new ExchangeRate(currencyPairClone, this.rate);
  }
}

ExchangeRate.invert = exchangeRate => {
  let invertedCurrencyPair = CurrencyPair.invert(exchangeRate.currencyPair);
  let invertedRate = exchangeRate.rate.inverse();
  return new ExchangeRate(invertedCurrencyPair, invertedRate);
};

ExchangeRate.convert = (amount, exchangeRate) => {
  return amount.multiply(exchangeRate.rate);
};

export default ExchangeRate;
