import StompConnectionManager from '../commons/stomp-connection-manager';

/** Spire risk service client class */
class RiskService {
  constructor() {
    this.stompConnectionManager = new StompConnectionManager('risk_service/portfolio');
  }

  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Market Data Service Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  subscribePortfolio(listener) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio';
    return this.stompConnectionManager.subscribe.apply(this.stompConnectionManager, [destination, listener]);
  }

  unsubscribe(subId) {
    this.stompConnectionManager.unsubscribe.apply(this.stompConnectionManager, [subId]);
  }

  setFilter(filter) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio/filter';
    this.stompConnectionManager.send.apply(this.stompConnectionManager, [destination, filter]);
  }
}

export default RiskService;