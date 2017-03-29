import StompConnectionManager from '../commons/stomp-connection-manager';
import DirectoryEntry from '../../definitions/directory-entry';
import Security from '../../definitions/security';
import CurrencyId from '../../definitions/currency/id';
import Money from '../../definitions/money';

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
    return this.stompConnectionManager.subscribe.apply(this.stompConnectionManager, [destination, onPortfolioMessageReceived.bind(this)]);

    function onPortfolioMessageReceived(message){
      let unmarshalledMessage = this.unmarshallPortfolioMessage(message);
      listener(unmarshalledMessage);
    }
  }

  /** @private */
  unmarshallPortfolioMessage(frameMessage) {
    let payload = JSON.parse(frameMessage.body);

    let account = DirectoryEntry.fromData(payload.account);
    payload.account = account;

    let security = Security.fromData(payload.security);
    payload.security = security;

    let currency = CurrencyId.fromNumber(payload.currency);
    payload.currency = currency;

    let averagePrice = Money.fromRepresentation(payload.average_price);
    payload.average_price = averagePrice;

    let costBasis = Money.fromRepresentation(payload.cost_basis);
    payload.cost_basis = costBasis;

    let fees = Money.fromRepresentation(payload.fees);
    payload.fees = fees;

    let realizedPnL = Money.fromRepresentation(payload.realized_profit_and_loss);
    payload.realized_profit_and_loss = realizedPnL;

    let totalPnL = Money.fromRepresentation(payload.total_profit_and_loss);
    payload.total_profit_and_loss = totalPnL;

    let unrealizedPnL = Money.fromRepresentation(payload.unrealized_profit_and_loss);
    payload.unrealized_profit_and_loss = unrealizedPnL;

    delete payload.__version;

    return payload;
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