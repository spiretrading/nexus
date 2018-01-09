import DirectoryEntry from '../../definitions/directory-entry';
import Security from '../../definitions/security';
import CurrencyId from '../../definitions/currency-id';
import Money from '../../definitions/money';
import HashMap from 'hashmap';

const THROTTLE_INTERVAL = 500;

export default class {
  constructor(listener) {
    this.listener = listener;
    this.messages = new HashMap();
    this.listenerRunner = setInterval(this.runListener.bind(this), THROTTLE_INTERVAL);
    this.runListener = this.runListener.bind(this);
  }

  handle(message) {
    let unmarshalledMessage = this.unmarshallPortfolioMessage(message);
    let messageKey = unmarshalledMessage.account.name + '.' +
      unmarshalledMessage.inventory.position.key.index.symbol + '.' +
      unmarshalledMessage.inventory.position.key.index.market.toData();
    this.messages.set(messageKey, unmarshalledMessage);
  }

  /** @private */
  unmarshallPortfolioMessage(frameMessage) {
    let payload = JSON.parse(frameMessage.body);

    let account = DirectoryEntry.fromData(payload.account);
    payload.account = account;

    payload.inventory.position.key.index = Security.fromData(payload.inventory.position.key.index);

    payload.inventory.position.key.currency = CurrencyId.fromData(payload.inventory.position.key.currency);

    payload.inventory.position.cost_basis = Money.fromRepresentation(payload.inventory.position.cost_basis);
    payload.inventory.position.cost_basis = payload.inventory.position.cost_basis || Money.fromNumber(0);

    payload.inventory.gross_profit_and_loss = Money.fromRepresentation(payload.inventory.gross_profit_and_loss);
    payload.inventory.gross_profit_and_loss = payload.inventory.gross_profit_and_loss || Money.fromNumber(0);

    payload.inventory.fees = Money.fromRepresentation(payload.inventory.fees);
    payload.inventory.fees = payload.inventory.fees || Money.fromNumber(0);

    if (payload.unrealized_profit_and_loss.is_initialized) {
      payload.unrealized_profit_and_loss.value = Money.fromRepresentation(payload.unrealized_profit_and_loss.value);
      payload.unrealized_profit_and_loss.value = payload.unrealized_profit_and_loss.value || Money.fromNumber(0);
    }

    delete payload.__version;

    return payload;
  }

  /** @private */
  runListener() {
    let values = this.messages.values();
    this.messages = new HashMap();
    this.listener(values);
  }

  dispose() {
    clearInterval(this.listenerRunner);
  }
}
