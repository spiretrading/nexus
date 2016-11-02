import httpConnectionManager from '../commons/http-connection-manager';

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
      .catch(this.logErrorAndThrow);
  }
}

export default MarketDataService;
