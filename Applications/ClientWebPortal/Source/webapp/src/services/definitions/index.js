import {
  AdministrationClient,
  DefinitionsServiceClient,
  CountryDatabase,
  CountryCode,
  CurrencyDatabase,
  CurrencyId
} from 'spire-client';
import HashMap from 'hashmap';

/** Various definitions queried from back-end */
class DefService {
  constructor() {
    this.adminClient = new AdministrationClient();
    this.definitionsServiceClient = new DefinitionsServiceClient();
    this.countryDatabase = new CountryDatabase();
    this.currencyDatabase = new CurrencyDatabase();
  }

  /** @private */
  loadCountries() {
    return this.definitionsServiceClient.loadCountryData.apply(this.definitionsServiceClient).then(onResponse.bind(this));

    function onResponse(countries) {
      for (let i=0; i<countries.length; i++) {
        this.countryDatabase.add(countries[i]);
      }
    }
  }

  /** @private */
  loadCurrencies() {
    return this.definitionsServiceClient.loadCurrencyData.apply(this.definitionsServiceClient).then(onResponse.bind(this));

    function onResponse(currencies) {
      for (let i=0; i<currencies.length; i++) {
        this.currencyDatabase.add(currencies[i]);
      }
    }
  }

  /** @private */
  loadEntitlements() {
    return this.adminClient.loadEntitlementsData.apply(this.adminClient).then(onResponse.bind(this));

    function onResponse(entitlements) {
      this.entitlements = entitlements;
    }
  }

  /** @private */
  loadComplianceRuleSchemas() {
    return this.definitionsServiceClient.loadComplianceRuleSchemas.apply(this.definitionsServiceClient)
      .then(onResponse.bind(this));

    function onResponse(ruleSchemas) {
      this.complianceRuleSchemas = ruleSchemas;
    }
  }

  /** @private */
  loadMarkets() {
    return this.definitionsServiceClient.loadMarketDatabase.apply(this.definitionsServiceClient)
      .then(onResponse.bind(this));

    function onResponse(response) {
      this.markets = new HashMap();
      for (let i=0; i<response.entries.length; i++) {
        let marketCode = response.entries[i].code;
        this.markets.set(marketCode, response.entries[i]);
      }
      this.markets.set('*', {
        code: '*',
        country_code: 65535,
        currency: 65535,
        description: '*',
        display_name: '*'
      });
    }
  }

  initialize() {
    return Promise.all([
      this.loadCountries.apply(this),
      this.loadCurrencies.apply(this),
      this.loadEntitlements.apply(this),
      this.loadComplianceRuleSchemas.apply(this),
      this.loadMarkets(this)
    ]);
  }

  getCountries() {
    return this.countryDatabase.entries();
  }

  getCountryThreeLetterCode(number) {
    let countryCode = CountryCode.fromNumber(number);
    return this.countryDatabase.fromCode(countryCode).threeLetterCode;
  }

  getCountryNumber(threeLetterCode) {
    let countryCode = this.countryDatabase.fromThreeLetterCode(threeLetterCode);
    return countryCode.toNumber();
  }

  getCountryName(number) {
    let countryCode = CountryCode.fromNumber(number);
    return this.countryDatabase.fromCode(countryCode).name;
  }

  doesCurrencyExist(id) {
    if (!(id instanceof CurrencyId)) {
      id = CurrencyId.fromNumber(id);
    }
    return this.currencyDatabase.fromId(id) != null;
  }

  getAllCurrencyCodes() {
    let codes = [];
    let entries = this.currencyDatabase.entries();
    for (let i=0; i<entries.length; i++) {
      let entry = entries[i];
      codes.push(entry.code);
    }
    return codes;
  }

  getAllCurrencies() {
    return this.currencyDatabase.entries();
  }

  getCurrencyCode(id) {
    if (!(id instanceof CurrencyId)) {
      id = CurrencyId.fromNumber(id);
    }
    return this.currencyDatabase.fromId(id).code;
  }

  getCurrencySign(id) {
    if (!(id instanceof CurrencyId)) {
      id = CurrencyId.fromNumber(id);
    }
    let currencyEntry = this.currencyDatabase.fromId(id);
    if (currencyEntry != null){
      return currencyEntry.sign;
    } else {
      return '';
    }
  }

  getCurrencyNumber(code) {
    return this.currencyDatabase.fromCode(code).id.toNumber();
  }

  getEntitlements() {
    return this.entitlements;
  }

  getComplianceRuleSchemas() {
    return this.complianceRuleSchemas;
  }

  getComplianceRuleScehma(schemaName) {
    for (let i=0; i<this.complianceRuleSchemas.length; i++) {
      if (this.complianceRuleSchemas[i].name === schemaName) {
        return this.complianceRuleSchemas[i];
      }
    }
    return null;
  }

  getMarket(marketCode) {
    return this.markets.get(marketCode);
  }
}

export default new DefService();
