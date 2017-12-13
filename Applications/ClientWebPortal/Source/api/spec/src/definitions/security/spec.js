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
    expect(security.country.toNumber()).toBe(124);
    expect(security.market.toData()).toBe('XTSE');
    expect('XIU').toBe(security.symbol);
  });

  it("Get wild card security.", function() {
    let wildCardSecurity = Security.getWildCard();
    expect('*').toBe(wildCardSecurity.market.toData());
    expect('*').toBe(wildCardSecurity.symbol);
  });

  it("Is wild card check.", function() {
    expect(true).toBe(Security.isWildCard('*'));
    expect(true).toBe(Security.isWildCard('*.*'));
    expect(true).toBe(Security.isWildCard('*.*.*'));
    expect(false).toBe(Security.isWildCard('*.'));
    expect(false).toBe(Security.isWildCard('XIU'));
  });

  it("toString check", function() {
    let marketDatabaseEntry = {
      displayName: 'TSX'
    };
    let marketDatabase = {
      fromMarketCode: function(marketCode) {
        return marketDatabaseEntry;
      }
    };
    let security = Security.fromData(this.securityData);
    expect('XIU.TSX').toBe(security.toString(marketDatabase));
  });
});
