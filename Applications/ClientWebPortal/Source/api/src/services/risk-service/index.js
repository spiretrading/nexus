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

    payload.inventory.position.key.index = Security.fromData(payload.inventory.position.key.index);

    payload.inventory.position.key.currency = CurrencyId.fromNumber(payload.inventory.position.key.currency);

    payload.inventory.position.cost_basis = Money.fromRepresentation(payload.inventory.position.cost_basis);

    payload.inventory.gross_profit_and_loss = Money.fromRepresentation(payload.inventory.gross_profit_and_loss);

    payload.inventory.fees = Money.fromRepresentation(payload.inventory.fees);

    if (payload.unrealized_profit_and_loss.is_initialized) {
      payload.unrealized_profit_and_loss.value = Money.fromRepresentation(payload.unrealized_profit_and_loss.value);
    }

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