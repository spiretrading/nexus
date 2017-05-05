'use strict';

var _exchangeRate = require('../../../../dist/definitions/exchange-rate');

var _exchangeRate2 = _interopRequireDefault(_exchangeRate);

var _money = require('../../../../dist/definitions/money');

var _money2 = _interopRequireDefault(_money);

var _exchangeRateTable = require('../../../../dist/definitions/exchange-rate-table');

var _exchangeRateTable2 = _interopRequireDefault(_exchangeRateTable);

var _currencyDatabase = require('../../../../dist/definitions/currency-database');

var _currencyDatabase2 = _interopRequireDefault(_currencyDatabase);

var _currencyDatabaseEntry = require('../../../../dist/definitions/currency-database-entry');

var _currencyDatabaseEntry2 = _interopRequireDefault(_currencyDatabaseEntry);

var _currencyId = require('../../../../dist/definitions/currency-id');

var _currencyId2 = _interopRequireDefault(_currencyId);

var _currencyPair = require('../../../../dist/definitions/currency-pair');

var _currencyPair2 = _interopRequireDefault(_currencyPair);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

describe("ExchangeRateTable", function () {
  beforeAll(function () {
    // initialize an exchange rate
    var usdEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(840), 'USD', '$');
    var cadEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(124), 'CAD', '$');
    this.currencyDatabase = new _currencyDatabase2.default();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
    this.currencyPair = _currencyPair2.default.parse('USD/CAD', this.currencyDatabase);
    this.exchangeRate = new _exchangeRate2.default(this.currencyPair, 0.25);

    this.exchangeRateTable = new _exchangeRateTable2.default();
  });

  it("Add an exchange rate", function () {
    this.exchangeRateTable.add.apply(this.exchangeRateTable, [this.exchangeRate]);
    var counterExchangeRates = this.exchangeRateTable.exchangeRates.get(840);
    var exchangeRate = counterExchangeRates.get(124);
    expect(0.25).toBe(exchangeRate.rate);
  });

  it("Find an exchange rate - currency in right order", function () {
    var baseCurrencyId = new _currencyId2.default(840);
    var counterCurrencyId = new _currencyId2.default(124);
    var currencyPair = new _currencyPair2.default(baseCurrencyId, counterCurrencyId);
    var exchangeRate = this.exchangeRateTable.find.apply(this.exchangeRateTable, [currencyPair]);
    expect(0.25).toBe(exchangeRate.rate);
    expect(840).toBe(exchangeRate.currencyPair.base.toNumber());
    expect(124).toBe(exchangeRate.currencyPair.counter.toNumber());
  });

  it("Find an exchange rate - currency in reverse order", function () {
    var baseCurrencyId = new _currencyId2.default(124);
    var counterCurrencyId = new _currencyId2.default(840);
    var currencyPair = new _currencyPair2.default(baseCurrencyId, counterCurrencyId);
    var exchangeRate = this.exchangeRateTable.find.apply(this.exchangeRateTable, [currencyPair]);
    expect(4).toBe(exchangeRate.rate);
    expect(124).toBe(exchangeRate.currencyPair.base.toNumber());
    expect(840).toBe(exchangeRate.currencyPair.counter.toNumber());
  });
});
//# sourceMappingURL=spec.js.map
