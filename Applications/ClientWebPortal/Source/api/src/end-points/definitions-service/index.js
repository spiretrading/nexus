import httpConnectionManager from '../commons/http-connection-manager';

/** Spire definitions service client class */
class DefinitionsService {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Definitions Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
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

  loadComplianceRuleSchemas() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'definitions_service/load_compliance_rule_schemas';

    return httpConnectionManager.send(apiPath, null, true)
      .catch(this.logErrorAndThrow);
  }

  loadMarketDatabase() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'definitions_service/load_market_database';

    return httpConnectionManager.send(apiPath, null, true)
      .catch(this.logErrorAndThrow);
  }
}

export default DefinitionsService;
