import ExchangeRate from '../../../../dist/definitions/exchange-rate';
import Money from '../../../../dist/definitions/money';
import CurrencyPair from '../../../../dist/definitions/currency-pair';
import CurrencyDatabase from '../../../../dist/definitions/currency-database';
import CurrencyDatabaseEntry from '../../../../dist/definitions/currency-database-entry';
import CurrencyId from '../../../../dist/definitions/currency-id';

describe("ExchangeRate", function() {
  beforeAll(function() {
    let usdEntry = new CurrencyDatabaseEntry(new CurrencyId(840), 'USD', '$');
    let cadEntry = new CurrencyDatabaseEntry(new CurrencyId(124), 'CAD', '$');
    this.currencyDatabase = new CurrencyDatabase();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
    let currencyPair = CurrencyPair.parse('USD/CAD', this.currencyDatabase);
    this.exchangeRate = new ExchangeRate(currencyPair, 0.25);
  });

  it("Invert exchange rate.", function() {
    let invertedRate = ExchangeRate.invert(this.exchangeRate);
    expect(124).toBe(invertedRate.currencyPair.base.toNumber());
    expect(840).toBe(invertedRate.currencyPair.counter.toNumber());
    expect(4).toBe(invertedRate.rate);
  });

  it("Convert money.", function() {
    let money = Money.fromNumber(100);
    let convertedAmount = ExchangeRate.convert(money, this.exchangeRate);
    expect(25).toBe(convertedAmount.toNumber());
  });
});
