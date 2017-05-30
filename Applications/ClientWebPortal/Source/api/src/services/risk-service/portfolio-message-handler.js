import DirectoryEntry from '../../definitions/directory-entry';
import Security from '../../definitions/security';
import CurrencyId from '../../definitions/currency-id';
import Money from '../../definitions/money';
import HashMap from 'hashmap';

const THROTTLE_INTERVAL = 500;

class PortfolioMessageHandler {
  constructor(listener) {
    this.listener = listener;
    this.messages = new HashMap();
    this.listenerRunner = setInterval(this.runListener.bind(this), THROTTLE_INTERVAL);
  }

  handle(message) {
    let unmarshalledMessage = this.unmarshallPortfolioMessage(message);
    let messageKey = unmarshalledMessage.account.name.toString() + '.' +
      unmarshalledMessage.inventory.position.key.index.symbol + '.' +
      unmarshalledMessage.inventory.position.key.index.market.toCode();
    this.messages.set(messageKey, unmarshalledMessage);
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

  /** @private */
  runListener() {
    this.listener(this.messages.values());
    this.messages.clear();
  }

  dispose() {
    clearInterval(this.listenerRunner);
  }
}

export default PortfolioMessageHandler;
