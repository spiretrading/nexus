import {
  AdministrationClient,
  DefinitionsServiceClient,
  CountryDatabase,
  CountryCode,
  CurrencyDatabase,
  CurrencyId,
  MarketCode,
  MarketDatabase,
  MarketDatabaseEntry
} from 'spire-client';
import HashMap from 'hashmap';

/** Various definitions queried from back-end */
class DefService {
  constructor() {
    this.adminClient = new AdministrationClient();
    this.definitionsServiceClient = new DefinitionsServiceClient();
    this.countryDatabase = new CountryDatabase();
    this.currencyDatabase = new CurrencyDatabase();
    this.marketDatabase = new MarketDatabase();
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
      for (let i=0; i<response.entries.length; i++) {
        let marketDatabaseEntry = new MarketDatabaseEntry(
          new MarketCode(response.entries[i].code),
          CountryCode.fromNumber(response.entries[i].country_code),
          response.entries[i].time_zone,
          CurrencyId.fromNumber(response.entries[i].currency),
          response.entries[i].board_lot,
          response.entries[i].description,
          response.entries[i].display_name
        );
        this.marketDatabase.add.apply(this.marketDatabase, [marketDatabaseEntry]);
      }

      this.marketDatabase.add.apply(this.marketDatabase, [new MarketDatabaseEntry(
        new MarketCode('*'),
        CountryCode.fromNumber(65535),
        null,
        CurrencyId.fromNumber(65535),
        null,
        '*',
        '*'
      )]);
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
    return this.countryDatabase.fromCode.apply(this.countryDatabase, [number]).name;
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

  getCurrencyId(code) {
    return this.currencyDatabase.fromCode(code).id;
  }

  getCurrencySignFromId(id) {
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

  getCurrencySignFromCode(code) {
    return this.currencyDatabase.fromCode(code).sign;
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
        return clone(this.complianceRuleSchemas[i]);
      }
    }
    return null;
  }

  getMarket(marketCode) {
    return this.marketDatabase.fromMarketCode.apply(this.marketDatabase, [marketCode]);
  }

  getAllMarkets() {
    return this.marketDatabase.entries.apply(this.marketDatabase);
  }

  getMarketDatabase() {
    return this.marketDatabase;
  }
}

export default new DefService();
