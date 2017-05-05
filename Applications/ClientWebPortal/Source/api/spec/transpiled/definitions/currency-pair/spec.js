'use strict';

var _currencyPair = require('../../../../dist/definitions/currency-pair');

var _currencyPair2 = _interopRequireDefault(_currencyPair);

var _currencyDatabase = require('../../../../dist/definitions/currency-database');

var _currencyDatabase2 = _interopRequireDefault(_currencyDatabase);

var _currencyDatabaseEntry = require('../../../../dist/definitions/currency-database-entry');

var _currencyDatabaseEntry2 = _interopRequireDefault(_currencyDatabaseEntry);

var _currencyId = require('../../../../dist/definitions/currency-id');

var _currencyId2 = _interopRequireDefault(_currencyId);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

describe("CurrencyPair", function () {
  beforeAll(function () {
    var usdEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(840), 'USD', '$');
    var cadEntry = new _currencyDatabaseEntry2.default(new _currencyId2.default(124), 'CAD', '$');
    this.currencyDatabase = new _currencyDatabase2.default();
    this.currencyDatabase.add(usdEntry);
    this.currencyDatabase.add(cadEntry);
  });

  it("Parse symbol - valid case.", function () {
    var currencyPair = _currencyPair2.default.parse('USD/CAD', this.currencyDatabase);
    expect(840).toBe(currencyPair.base.toNumber());
    expect(124).toBe(currencyPair.counter.toNumber());
  });

  it("Parse symbol - invalid cases.", function () {
    var testSubject = function () {
      _currencyPair2.default.parse('USDCAD', this.currencyDatabase);
    }.bind(this);
    expect(testSubject).toThrow(new Error('Missing separator "/"'));

    testSubject = function () {
      _currencyPair2.default.parse('GBP/CAD', this.currencyDatabase);
    }.bind(this);
    expect(testSubject).toThrow(new Error('Invalid base currency code'));

    testSubject = function () {
      _currencyPair2.default.parse('USD/GBP', this.currencyDatabase);
    }.bind(this);
    expect(testSubject).toThrow(new Error('Invalid counter currency code'));
  });

  it("Invert currency pair.", function () {
    var pair = _currencyPair2.default.parse('USD/CAD', this.currencyDatabase);
    var invertedPair = _currencyPair2.default.invert(pair);
    expect(124).toBe(invertedPair.base.toNumber());
    expect(840).toBe(invertedPair.counter.toNumber());
  });
});
//# sourceMappingURL=spec.js.map
