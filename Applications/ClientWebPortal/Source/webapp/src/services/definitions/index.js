import definitionsServiceClient from 'utils/spire-clients/definitions-service';
import HashMap from 'hashmap';

/** Various definitions queried from back-end */
class DefinitionsService {
  /** @private */
  loadCountries() {
    return definitionsServiceClient.loadCountryData.apply(definitionsServiceClient).then(onResponse.bind(this));

    function onResponse(countries) {
      this.countries = countries;
      this.countriesByNumber = new HashMap();
      this.countriesByThreeLetterCode = new HashMap();
      for (let i=0; i<countries.length; i++) {
        let country = countries[i];
        this.countriesByNumber.set(country.code, country);
        this.countriesByThreeLetterCode.set(country.threeLetterCode, country);
      }
    }
  }

  /** @private */
  loadCurrencies() {
    return definitionsServiceClient.loadCurrencyData.apply(definitionsServiceClient).then(onResponse.bind(this));

    function onResponse(currencies) {
      this.currencies = currencies;
      this.currenciesById = new HashMap();
      this.currenciesByCode = new HashMap();
      for (let i=0; i<currencies.length; i++) {
        let currency = currencies[i];
        this.currenciesById.set(currency.id, currency);
        this.currenciesByCode.set(currency.code, currency);
      }
    }
  }

  initialize() {
    return Promise.all([
      this.loadCountries.apply(this),
      this.loadCurrencies.apply(this)
    ]);
  }

  getCountries() {
    return this.countries;
  }

  getCountryThreeLetterCode(number) {
    return this.countriesByNumber.get(number).threeLetterCode;
  }

  getCountryNumber(threeLetterCode) {
    return this.countriesByThreeLetterCode.get(threeLetterCode).code;
  }

  getCountryName(number) {
    return this.countriesByNumber.get(number).name;
  }

  getAllCurrencyCodes() {
    let codes = [];
    for (let i=0; i<this.currencies.length; i++) {
      codes.push(this.currencies[i].code);
    }
    return codes;
  }

  getCurrencyCode(id) {
    return this.currenciesById.get(id).code;
  }

  getCurrencySign(id) {
    return this.currenciesById.get(id).sign;
  }

  getCurrencyNumber(code) {
    return this.currenciesByCode.get(code).id;
  }
}

export default new DefinitionsService();
