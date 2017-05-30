import httpConnectionManager from '../commons/http-connection-manager';
import Security from '../../definitions/security';
import CountryCode from '../../definitions/country-code';
import MarketCode from '../../definitions/market-code';

/** Spire compliance service client class */
class MarketDataService {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Market Data Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  loadSecurityInfoFromPrefix(prefix) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'market_data_service/load_security_info_from_prefix';
    let payload = {
      prefix: prefix
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then((results) => {
        for (let i=0; i<results.length; i++) {
          results[i].security = new Security(
            new CountryCode(results[i].security.country),
            new MarketCode(results[i].security.market),
            results[i].security.symbol
          );
        }
        return results;
      })
      .catch(this.logErrorAndThrow);
  }
}

export default MarketDataService;
