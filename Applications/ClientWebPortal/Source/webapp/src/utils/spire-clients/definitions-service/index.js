import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
const ResultCode = ResultCodes;

/** Spire admin client class */
class DefinitionsServiceClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Spire Admin Client: Unexpected error happened.');
    console.debug(xhr);
    throw ResultCode.ERROR;
  }

  loadCountryData() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'definitions_service/load_country_database';

    return httpConnectionManager.send(apiPath, null, true)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      let countries = [];
      for (let i=0; i<response.entries.length; i++) {
        let country = response.entries[i];
        countries.push({
          code: country.code,
          name: country.name,
          threeLetterCode: country.three_letter_code,
          twoLetterCode: country.two_letter_code
        });
      }
      return countries;
    }
  }

  loadCurrencyData() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'definitions_service/load_currency_database';

    return httpConnectionManager.send(apiPath, null, true)
      .then(parseResponse)
      .catch(this.logErrorAndThrow);

    function parseResponse(response) {
      let currencies = [];
      for (let i=0; i<response.entries.length; i++) {
        let currency = response.entries[i];
        currencies.push({
          code: currency.code,
          id: currency.id,
          sign: currency.sign
        });
      }
      return currencies;
    }
  }
}

export default new DefinitionsServiceClient();
