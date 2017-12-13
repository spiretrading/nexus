import {
  AdministrationClient,
  DefinitionsServiceClient,
  CountryDatabase,
  CountryCode,
  CurrencyDatabase,
  CurrencyPair,
  CurrencyId,
  MarketCode,
  MarketDatabase,
  MarketDatabaseEntry,
  ExchangeRate,
  ExchangeRateTable
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
    this.exchangeRateTable = new ExchangeRateTable();

    this.loadExchangeRates = this.loadExchangeRates.bind(this);
  }

  /** @private */
  loadCountries() {
    return this.definitionsServiceClient.loadCountryData().then(onResponse.bind(this));

    function onResponse(countries) {
      for (let i=0; i<countries.length; i++) {
        this.countryDatabase.add(countries[i]);
      }
    }
  }

  /** @private */
  loadCurrencies() {
    return this.definitionsServiceClient.loadCurrencyData().then(onResponse.bind(this));

    function onResponse(currencies) {
      for (let i=0; i<currencies.length; i++) {
        this.currencyDatabase.add(currencies[i]);
      }
    }
  }

  /** @private */
  loadEntitlements() {
    return this.adminClient.loadEntitlementsData().then(onResponse.bind(this));

    function onResponse(entitlements) {
      this.entitlements = entitlements;
    }
  }

  /** @private */
  loadComplianceRuleSchemas() {
    return this.definitionsServiceClient.loadComplianceRuleSchemas()
      .then(onResponse.bind(this));

    function onResponse(ruleSchemas) {
      this.complianceRuleSchemas = ruleSchemas;
    }
  }

  /** @private */
  loadMarkets() {
    return this.definitionsServiceClient.loadMarketDatabase()
      .then(onResponse.bind(this));

    function onResponse(response) {
      for (let i=0; i<response.entries.length; i++) {
        let marketDatabaseEntry = new MarketDatabaseEntry(
          new MarketCode(response.entries[i].code),
          CountryCode.fromNumber(response.entries[i].country_code),
          response.entries[i].time_zone,
          CurrencyId.fromData(response.entries[i].currency),
          response.entries[i].board_lot,
          response.entries[i].description,
          response.entries[i].display_name
        );
        this.marketDatabase.add(marketDatabaseEntry);
      }

      this.marketDatabase.add(new MarketDatabaseEntry(
        new MarketCode('*'),
        CountryCode.fromNumber(65535),
        null,
        CurrencyId.fromData(65535),
        null,
        '*',
        '*'
      ));
    }
  }

  /** @private */
  loadExchangeRates() {
    return this.definitionsServiceClient.loadExchangeRates().then(onResponse.bind(this));

    function onResponse(exchangeRates) {
      for (let i=0; i<exchangeRates.length; i++) {
        let baseCurrencyId = CurrencyId.fromData(exchangeRates[i].pair.base);
        let baseCurrencyDatabaseEntry = this.currencyDatabase.fromId(baseCurrencyId);
        let counterCurrencyId = CurrencyId.fromData(exchangeRates[i].pair.counter);
        let counterCurrencyDatabaseEntry = this.currencyDatabase.fromId(counterCurrencyId);
        let pairCurrencyCode = baseCurrencyDatabaseEntry.code + '/' + counterCurrencyDatabaseEntry.code;
        let currencyPair = CurrencyPair.parse(pairCurrencyCode, this.currencyDatabase);
        let rateFraction = exchangeRates[i].rate.numerator + '/' + exchangeRates[i].rate.denominator;
        let exchangeRate = new ExchangeRate(currencyPair, rateFraction);
        this.exchangeRateTable.add(exchangeRate);
      }
    }
  }

  initialize() {
    let loadCurrenciesAndExchangeRates = this.loadCurrencies()
      .then(this.loadExchangeRates);

    return Promise.all([
      this.loadCountries(),
      loadCurrenciesAndExchangeRates,
      this.loadEntitlements(),
      this.loadComplianceRuleSchemas(),
      this.loadMarkets()
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
    return this.countryDatabase.fromCode(number).name;
  }

  doesCurrencyExist(id) {
    if (!(id instanceof CurrencyId)) {
      id = CurrencyId.fromData(id);
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
      id = CurrencyId.fromData(id);
    }
    return this.currencyDatabase.fromId(id).code;
  }

  getCurrencyId(code) {
    return this.currencyDatabase.fromCode(code).id;
  }

  getCurrencySignFromId(id) {
    if (!(id instanceof CurrencyId)) {
      id = CurrencyId.fromData(id);
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
    return this.marketDatabase.fromMarketCode(marketCode);
  }

  getAllMarkets() {
    return this.marketDatabase.entries();
  }

  getMarketDatabase() {
    return this.marketDatabase;
  }

  getExchangeRateTable() {
    return this.exchangeRateTable;
  }
}

export default new DefService();
