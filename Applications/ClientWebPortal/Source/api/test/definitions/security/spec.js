import Security from 'dist/definitions/security/index.js';

describe("Security", function() {

  beforeAll(function() {
    this.securityData = {
      country: 124,
      market: 'XTSE',
      symbol: 'XIU'
    };


  });

  it("and so is a spec", function() {
    var security = new Security(this.securityData);
    console.log(security);

    var a = true;

    expect(a).toBe(true);
  });
});