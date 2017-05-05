import CurrencyPair from '../currency-pair';

class ExchangeRate {
  constructor(currencyPair, rate) {
    this.currencyPair = currencyPair;
    this.rate = rate;
  }

  clone() {
    let currencyPairClone = this.currencyPair.clone.apply(this.currencyPair);
    return new ExchangeRate(currencyPairClone, this.rate);
  }
}

ExchangeRate.invert = exchangeRate => {
  let invertedCurrencyPair = CurrencyPair.invert(exchangeRate.currencyPair);
  let invertedRate = 1 / exchangeRate.rate;
  return new ExchangeRate(invertedCurrencyPair, invertedRate);
};

ExchangeRate.convert = (amount, exchangeRate) => {
  return amount.multiply(exchangeRate.rate);
};

export default ExchangeRate;
