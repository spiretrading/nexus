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

  setPortfolioDataFilter(subId, filter) {
    let destination = Config.BACKEND_API_ROOT_URL + 'risk_service/portfolio/filter';

    let serializedFilter = {
      id: subId,
      groups: [],
      currencies: [],
      markets: []
    };

    for (let i=0; i<filter.groups.length; i++) {
      serializedFilter.groups.push(filter.groups[i].toData());
    }

    for (let i=0; i<filter.currencies.length; i++) {
      serializedFilter.currencies.push(filter.currencies[i].toData());
    }

    for (let i=0; i<filter.markets.length; i++) {
      serializedFilter.markets.push(filter.markets[i].toData());
    }

    this.stompConnectionManager.send(destination, serializedFilter);
  }
}

export default RiskService;
