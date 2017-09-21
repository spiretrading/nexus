import DirectoryEntry from '../../definitions/directory-entry';
import Security from '../../definitions/security';
import CurrencyId from '../../definitions/currency-id';
import Money from '../../definitions/money';
import HashMap from 'hashmap';

// const THROTTLE_INTERVAL = 500;
// TODO: remove this temporary code
const THROTTLE_INTERVAL = 500;

export default class {
  constructor(listener) {
    this.listener = listener;
    this.messages = new HashMap();
    this.listenerRunner = setInterval(this.runListener.bind(this), THROTTLE_INTERVAL);

    // TODO: remove this temporary code
    setInterval(this.runRandomDataGenerator.bind(this), 400);
    this.randomAccountCache = new HashMap();
    this.randomAccountNameCache = new HashMap();
    this.counter = 0;
  }

  handle(message) {
    let unmarshalledMessage = this.unmarshallPortfolioMessage(message);
    let messageKey = unmarshalledMessage.account.name.toString() + '.' +
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

    payload.inventory.gross_profit_and_loss = Money.fromRepresentation(payload.inventory.gross_profit_and_loss);

    payload.inventory.fees = Money.fromRepresentation(payload.inventory.fees);

    if (payload.unrealized_profit_and_loss.is_initialized) {
      payload.unrealized_profit_and_loss.value = Money.fromRepresentation(payload.unrealized_profit_and_loss.value);
    }

    delete payload.__version;

    return payload;
  }

  // TODO: remove this temporary code
  runRandomDataGenerator() {
    let unmarshalledMessage = this.randomlyGenerateData();
    if (unmarshalledMessage != null) {
      let messageKey = unmarshalledMessage.account.name.toString() + '.' +
        unmarshalledMessage.inventory.position.key.index.symbol + '.' +
        unmarshalledMessage.inventory.position.key.index.market.toData();
      this.messages.set(messageKey, unmarshalledMessage);
    }
  }

  // TODO: remove this temporary code
  randomlyGenerateData() {
    let payload = {};

    let randomId = this.randomIntegerGenerator(1,5000);
    let directoryEntry;
    if (this.randomAccountCache.has(randomId)) {
      directoryEntry = this.randomAccountCache.get(randomId);
    } else {
      let name = this.randomlyGeneratedString();
      if (this.randomAccountNameCache.has(name)) {
        return null;
      } else {
        this.randomAccountNameCache.set(name, true);
      }
      directoryEntry = new DirectoryEntry(randomId, 0, name);
      this.randomAccountCache.set(randomId, directoryEntry);
    }

    payload.account = directoryEntry;
    payload.inventory = {
      fees: Money.fromRepresentation(this.randomMoneyNumberGenerator(2, 10)),
      gross_profit_and_loss: Money.fromRepresentation(this.randomMoneyNumberGenerator(300, 1000)),
      position: {
        cost_basis: Money.fromRepresentation(this.randomMoneyNumberGenerator(10000, 50000)),
        key: {
          currency: this.randomCurrencyIdGenerator(),
          index: this.randomSecurityGenerator()
        },
        quantity: this.randomIntegerGenerator(100, 10000)
      },
      transaction_count: this.randomIntegerGenerator(1, 100),
      volume: this.randomIntegerGenerator(100, 10000)
    };
    payload.unrealized_profit_and_loss = {
      is_initialized: true,
      value: Money.fromRepresentation(this.randomMoneyNumberGenerator(300, 1000))
    };

    return payload;
  }

  // TODO: remove this temporary code
  randomlyGeneratedString() {
    var text = "";
    var possible = "ABCDE";

    for (var i = 0; i < 5; i++)
      text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
  }

  // TODO: remove this temporary code
  randomMoneyNumberGenerator(minDollars, maxDollars) {
    minDollars = minDollars * 100;
    maxDollars = maxDollars * 100;
    var ranNum = Math.floor(Math.random() * (maxDollars - minDollars + 1)) + minDollars;
    return ranNum * 10000;
  }

  // TODO: remove this temporary code
  randomIntegerGenerator(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
  }

  // TODO: remove this temporary code
  randomCurrencyIdGenerator() {
    let randomNum = this.randomIntegerGenerator(1,2);
    if (randomNum == 1) {
      // cad
      return CurrencyId.fromData(124);
    } else {
      // usd
      return CurrencyId.fromData(840);
    }
  }

  // TODO: remove this temporary code
  randomSecurityGenerator() {
    let randomNum = this.randomIntegerGenerator(1,2);
    let countryCode;
    if (randomNum == 1) {
      // cad
      countryCode = 124;
    } else {
      // usd
      countryCode = 840;
    }

    let data = {
      country: countryCode,
      market: 'XTSE',
      symbol: 'ABX'
    };

    return Security.fromData(data);
  }

  /** @private */
  runListener() {
    // TODO: remove this temporary code
    if (this.counter < 30) {
      this.randomlyGenerateData();

      this.listener(this.messages.values());
      this.messages.clear();
      this.counter++;
    }

  }

  dispose() {
    clearInterval(this.listenerRunner);
  }
}
