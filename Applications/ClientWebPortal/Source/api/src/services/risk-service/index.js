import StompConnectionManager from '../commons/stomp-connection-manager';
import PortfolioMessageHandler from './portfolio-message-handler';
import HashMap from 'hashmap';

/** Spire risk service client class */
class RiskService {
  constructor() {
    this.stompConnectionManager = new StompConnectionManager('risk_service/portfolio');
    this.portfolioMessageHandlers = new HashMap();
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
    return this.stompConnectionManager.subscribe.apply(
      this.stompConnectionManager,
      [destination, portfolioMsgHandler.handle.bind(portfolioMsgHandler)]
    ).then(subId => {
      this.portfolioMessageHandlers.set(subId, portfolioMsgHandler);
      return subId;
    });
  }

  unsubscribe(subId) {
    this.stompConnectionManager.unsubscribe.apply(this.stompConnectionManager, [subId]);
    let portfolioMsgHandler = this.portfolioMessageHandlers.get(subId);
    portfolioMsgHandler.dispose.apply(portfolioMsgHandler);
    this.portfolioMessageHandlers.remove(subId);
  }

  setFilter(filter) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio/filter';
    this.stompConnectionManager.send.apply(this.stompConnectionManager, [destination, filter]);
  }
}

export default RiskService;