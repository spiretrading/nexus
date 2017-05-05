'use strict';

var _exchangeRate = require('../../../../dist/definitions/exchange-rate');

var _exchangeRate2 = _interopRequireDefault(_exchangeRate);

var _money = require('../../../../dist/definitions/money');

var _money2 = _interopRequireDefault(_money);

var _currencyPair = require('../../../../dist/definitions/currency-pair');

var _currencyPair2 = _interopRequireDefault(_currencyPair);

var _currencyDatabase = require('../../../../dist/definitions/currency-database');

var _currencyDatabase2 = _interopRequireDefault(_currencyDatabase);

var _currencyDatabaseEntry = require('../../../../dist/definitions/currency-database-entry');

var _currencyDatabaseEntry2 = _interopRequireDefault(_currencyDatabaseEntry);

var _currencyId = require('../../../../dist/definitions/currency-id');

var _currencyId2 = _interopRequireDefault(_currencyId);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

describe("ExchangeRate", function () {
  beforeAll(function () {
    var usdEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(840), 'USD', '$');
    var cadEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(124), 'CAD', '$');
    this.currencyDatabase = new _currencyDatabase2.default();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
    var currencyPair = _currencyPair2.default.parse('USD/CAD', this.currencyDatabase);
    this.exchangeRate = new _exchangeRate2.default(currencyPair, 0.25);
  });

  it("Invert exchange rate.", function () {
    var invertedRate = _exchangeRate2.default.invert(this.exchangeRate);
    expect(124).toBe(invertedRate.currencyPair.base.toNumber());
    expect(840).toBe(invertedRate.currencyPair.counter.toNumber());
    expect(4).toBe(invertedRate.rate);
  });

  it("Convert money.", function () {
    var money = _money2.default.fromNumber(100);
    var convertedAmount = _exchangeRate2.default.convert(money, this.exchangeRate);
    expect(25).toBe(convertedAmount.toNumber());
  });
});
//# sourceMappingURL=spec.js.map
