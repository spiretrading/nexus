'use strict';

var _security = require('../../../../dist/definitions/security');

var _security2 = _interopRequireDefault(_security);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

describe("Security", function () {
  beforeAll(function () {
    this.securityData = {
      country: 124,
      market: 'XTSE',
      symbol: 'XIU'
    };
  });

  it("Convert to Security from plain object.", function () {
    var security = _security2.default.fromData(this.securityData);
    expect('Security').toBe(security.constructor.name);
    expect('CountryCode').toBe(security.country.constructor.name);
    expect('MarketCode').toBe(security.market.constructor.name);
    expect('XIU').toBe(security.symbol);
  });

  it("Get wild card security.", function () {
    var wildCardSecurity = _security2.default.getWildCard();
    expect('*').toBe(wildCardSecurity.market.toCode());
    expect('*').toBe(wildCardSecurity.symbol);
  });

  it("Is wild card check.", function () {
    expect(true).toBe(_security2.default.isWildCard('*'));
    expect(true).toBe(_security2.default.isWildCard('*.*'));
    expect(true).toBe(_security2.default.isWildCard('*.*.*'));
    expect(false).toBe(_security2.default.isWildCard('*.'));
    expect(false).toBe(_security2.default.isWildCard('XIU'));
  });
});
//# sourceMappingURL=spec.js.map
