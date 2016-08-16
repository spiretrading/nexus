import definitionsServiceClient from 'utils/spire-clients/definitions-service';
import HashMap from 'hashmap';

/** Various definitions queried from back-end */
class DefinitionsService {
  /** @private */
  loadCountries() {
    return definitionsServiceClient.loadCountryData().then(onResponse.bind(this));

    function onResponse(countries) {
      this.countriesByNumber = new HashMap();
      this.countriesByThreeLetterCode = new HashMap();
      for (let i=0; i<countries.length; i++) {
        let country = countries[i];
        this.countriesByNumber.set(country.code, country);
        this.countriesByThreeLetterCode.set(country.threeLetterCode, country);
      }
    }
  }

  initialize() {
    return Promise.all([
      this.loadCountries.apply(this)
    ]);
  }

  getCountryThreeLetterCode(number) {
    return this.countriesByNumber.get(number).threeLetterCode;
  }

  getCountryNumber(threeLetterCode) {
    return this.countriesByThreeLetterCode.get(threeLetterCode).code;
  }
}

export default new DefinitionsService();
