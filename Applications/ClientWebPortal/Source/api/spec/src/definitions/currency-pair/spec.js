import CurrencyPair from '../../../../dist/definitions/currency-pair';
import CurrencyDatabase from '../../../../dist/definitions/currency-database';
import CurrencyDatabaseEntry from '../../../../dist/definitions/currency-database-entry';
import CurrencyId from '../../../../dist/definitions/currency-id';

describe("CurrencyPair", function() {
  beforeAll(function() {
    let usdEntry = new CurrencyDatabaseEntry(new CurrencyId(840), 'USD', '$');
    let cadEntry = new CurrencyDatabaseEntry(new CurrencyId(124), 'CAD', '$');
    this.currencyDatabase = new CurrencyDatabase();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
  });

  it("Parse symbol - valid case.", function() {
    let currencyPair = CurrencyPair.parse('USD/CAD', this.currencyDatabase);
    expect(840).toBe(currencyPair.base.toNumber());
    expect(124).toBe(currencyPair.counter.toNumber());
  });

  it("Parse symbol - invalid cases.", function() {
    let testSubject = function(){ CurrencyPair.parse('USDCAD', this.currencyDatabase); }.bind(this);
    expect(testSubject).toThrow(new Error('Missing separator "/"'));

    testSubject = function(){ CurrencyPair.parse('GBP/CAD', this.currencyDatabase); }.bind(this);
    expect(testSubject).toThrow(new Error('Invalid base currency code'));

    testSubject = function(){ CurrencyPair.parse('USD/GBP', this.currencyDatabase); }.bind(this);
    expect(testSubject).toThrow(new Error('Invalid counter currency code'));
  });

  it("Invert currency pair.", function() {
    let pair = CurrencyPair.parse('USD/CAD', this.currencyDatabase);
    let invertedPair = CurrencyPair.invert(pair);
    expect(124).toBe(invertedPair.base.toNumber());
    expect(840).toBe(invertedPair.counter.toNumber());
  });
});
