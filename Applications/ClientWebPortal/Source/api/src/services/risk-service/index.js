import StompConnectionManager from '../commons/stomp-connection-manager';
import PortfolioMessageHandler from './portfolio-message-handler';
import HashMap from 'hashmap';

/** Spire risk service client class */
class RiskService {
  constructor() {
    this.stompConnectionManager = new StompConnectionManager('risk_service/portfolio');
    this.portfolioMessageHandlers = new HashMap();

    this.unsubscribe = this.unsubscribe.bind(this);
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
    let portfolioMsgHandler = new PortfolioMessageHandler(listener);
    return this.stompConnectionManager.subscribe(
      destination, portfolioMsgHandler.handle.bind(portfolioMsgHandler)
    ).then(subId => {
      this.portfolioMessageHandlers.set(subId, portfolioMsgHandler);
      return subId;
    });
  }

  unsubscribe(subId) {
    this.stompConnectionManager.unsubscribe(subId);
    let portfolioMsgHandler = this.portfolioMessageHandlers.get(subId);
    portfolioMsgHandler.dispose();
    this.portfolioMessageHandlers.remove(subId);
  }

  setFilter(filter) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio/filter';
    this.stompConnectionManager.send(destination, filter);
  }
}

export default RiskService;
