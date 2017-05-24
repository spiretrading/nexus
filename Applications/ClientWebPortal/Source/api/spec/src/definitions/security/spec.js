import Security from '../../../../dist/definitions/security';

describe("Security", function() {
  beforeAll(function() {
    this.securityData = {
      country: 124,
      market: 'XTSE',
      symbol: 'XIU'
    };
  });

  it("Convert to Security from plain object.", function() {
    let security = Security.fromData(this.securityData);
    expect('Security').toBe(security.constructor.name);
    expect('CountryCode').toBe(security.country.constructor.name);
    expect('MarketCode').toBe(security.market.constructor.name);
    expect('XIU').toBe(security.symbol);
  });

  it("Get wild card security.", function() {
    let wildCardSecurity = Security.getWildCard();
    expect('*').toBe(wildCardSecurity.market.toCode());
    expect('*').toBe(wildCardSecurity.symbol);
  });

  it("Is wild card check.", function() {
    expect(true).toBe(Security.isWildCard('*'));
    expect(true).toBe(Security.isWildCard('*.*'));
    expect(true).toBe(Security.isWildCard('*.*.*'));
    expect(false).toBe(Security.isWildCard('*.'));
    expect(false).toBe(Security.isWildCard('XIU'));
  });
});