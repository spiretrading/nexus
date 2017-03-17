import stompConnectionManager from '../commons/stomp-connection-manager';

/** Spire risk service client class */
class RiskService {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Market Data Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  subscribePortfolio(listener) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio';
    return stompConnectionManager.subscribe.apply(stompConnectionManager, [destination, listener]);
  }

  unsubscribe(subId) {
    stompConnectionManager.unsubscribe.apply(stompConnectionManager, [subId]);
  }

  setFilter(filter) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio/filter';
    stompConnectionManager.send(destination, filter);
  }
}

export default RiskService;