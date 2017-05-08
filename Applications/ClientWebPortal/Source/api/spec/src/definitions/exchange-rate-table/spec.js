import ExchangeRate from '../../../../dist/definitions/exchange-rate';
import Money from '../../../../dist/definitions/money';
import ExchangeRateTable from '../../../../dist/definitions/exchange-rate-table';
import CurrencyDatabase from '../../../../dist/definitions/currency-database';
import CurrencyDatabaseEntry from '../../../../dist/definitions/currency-database-entry';
import CurrencyId from '../../../../dist/definitions/currency-id';
import CurrencyPair from '../../../../dist/definitions/currency-pair';

describe("ExchangeRateTable", function() {
  beforeAll(function() {
    // initialize an exchange rate
    let usdEntry = new CurrencyDatabaseEntry(new CurrencyId(840), 'USD', '$');
    let cadEntry = new CurrencyDatabaseEntry(new CurrencyId(124), 'CAD', '$');
    this.currencyDatabase = new CurrencyDatabase();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
    this.currencyPair = CurrencyPair.parse('USD/CAD', this.currencyDatabase);
    this.exchangeRate = new ExchangeRate(this.currencyPair, 0.25);

    this.exchangeRateTable = new ExchangeRateTable();
  });

  it("Add an exchange rate", function() {
    this.exchangeRateTable.add.apply(this.exchangeRateTable, [this.exchangeRate]);
    let counterExchangeRates = this.exchangeRateTable.exchangeRates.get(840);
    let exchangeRate = counterExchangeRates.get(124);
    expect(0.25).toBe(exchangeRate.rate.valueOf());
  });

  it("Find an exchange rate - currency in right order", function() {
    let baseCurrencyId = new CurrencyId(840);
    let counterCurrencyId = new CurrencyId(124);
    let currencyPair = new CurrencyPair(baseCurrencyId, counterCurrencyId);
    let exchangeRate = this.exchangeRateTable.find.apply(this.exchangeRateTable, [currencyPair]);
    expect(0.25).toBe(exchangeRate.rate.valueOf());
    expect(840).toBe(exchangeRate.currencyPair.base.toNumber());
    expect(124).toBe(exchangeRate.currencyPair.counter.toNumber());
  });

  it("Find an exchange rate - currency in reverse order", function() {
    let baseCurrencyId = new CurrencyId(124);
    let counterCurrencyId = new CurrencyId(840);
    let currencyPair = new CurrencyPair(baseCurrencyId, counterCurrencyId);
    let exchangeRate = this.exchangeRateTable.find.apply(this.exchangeRateTable, [currencyPair]);
    expect(4).toBe(exchangeRate.rate.valueOf());
    expect(124).toBe(exchangeRate.currencyPair.base.toNumber());
    expect(840).toBe(exchangeRate.currencyPair.counter.toNumber());
  });
});
