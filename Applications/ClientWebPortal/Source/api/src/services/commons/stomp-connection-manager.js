import HashMap from 'hashmap';
import uuid from 'uuid';
import webstomp from 'webstomp-client';

/** Mock Server - only committed during development. Will be removed when pull request is submitted */
class MockServer {
  constructor() {
    // test securities
    this.securities = [
      'WOW.TSX',
      'LOL.TSX',
      'DOTA.TSX',
    ];

    this.currencies = [
      'AUD',
      'CAD'
    ];

    this.accounts = [
      'hill319',
      'jimin123'
    ];
  }

  getRandomInt(inclusiveMin, inclusiveMax) {
    return Math.floor(Math.random() * (inclusiveMax - inclusiveMin + 1)) + inclusiveMin;
  }

  getRandomDecimals(min, max, decimals) {
    return Number((Math.random() * (max - min) + min).toFixed(decimals));
  }

  subscribe(destination, listener) {
    if (destination === '/api/risk_service/portfolio') {
      this.portfolioInterval = setInterval(() => {
        let portfolioData = this.generateRandomPortfolioData.apply(this);
        listener(portfolioData);
      }, 1000);
    }

    function generateSubscription(destination) {
      return {
        unsubscribe: function() {
          this.unsubscribe(destination);
        }.bind(this)
      };
    }

    return generateSubscription.apply(this, [destination]);
  }

  generateRandomPortfolioData() {
    let data = {};
    data.account = this.accounts[this.getRandomInt(0, this.accounts.length-1)];
    data.security = this.securities[this.getRandomInt(0, this.securities.length-1)];
    data.currency = this.currencies[this.getRandomInt(0, this.currencies.length-1)];
    data.open_quantity = this.getRandomInt(0, 5000);
    data.average_price = this.getRandomDecimals(0, 100, 2);
    data.total_profit_and_loss = this.getRandomDecimals(0, 1000, 2);
    data.unrealized_profit_and_loss = this.getRandomDecimals(0, 1000, 2);
    data.realized_profit_and_loss = this.getRandomDecimals(0, 1000, 2);
    data.fees = this.getRandomDecimals(0, 1000, 2);
    data.cost_basis = this.getRandomDecimals(0, 1000, 2);
    data.volume = this.getRandomInt(0, 1000);
    data.trades = this.getRandomInt(0, 1000);
    if (this.getRandomInt(0, 1) == 0) {
      data.total_profit_and_loss = (-1) * data.total_profit_and_loss;
      data.unrealized_profit_and_loss = (-1) * data.unrealized_profit_and_loss;
      data.realized_profit_and_loss = (-1) * data.realized_profit_and_loss;
    }
    return data;
  }

  unsubscribe(destination, listener) {
    if (destination === '/api/risk_service/portfolio') {
      clearInterval(this.portfolioInterval);
    }
  }

  send(destination, body) {
    if (destination === '/api/risk_service/portfolio/filter'){
      this.portfolioFilter = body;
    }
  }
}

/** Spire STOMP connection manager */
class StompConnectionManager {
  constructor(connectionPath) {
    this.connectionUrl = 'ws://' + window.location.host + Config.BACKEND_API_ROOT_URL + connectionPath;
    this.subIdToTopic = new HashMap();
    this.destinationToListeners = new HashMap();      // subId - listener callbacks
    this.destinationToSubscription = new HashMap();
  }

  connect() {
    // later to be replaced with the stomp client at
    // https://github.com/JSteunou/webstomp-client
    // this.client = new MockServer();
    /*
    return new Promise((resolve, reject) => {
      this.client = new MockServer();
      resolve();
    });
    */


    // below is a test code to see if a WS connection can be made
    // this.realClient = webstomp.client('ws://192.168.1.129:8080/api/risk_service/portfolio');
    return new Promise((resolve, reject) => {
      // TODO: temporarily hardcoded url
      let websocket = new WebSocket(this.connectionUrl);
      this.client = webstomp.over(websocket);
      this.client.connect({}, () => {
        console.debug('STOMP connection has been made.');
        resolve();
      });
    });

  }

  /** @private */
  onAllMessage(message) {
    let destination = this.destination;
    let listeners = this.connectionManager.destinationToListeners.get(destination).values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](message);
    }
  }

  /** @private */
  isConnected() {
    return this.client != null;
  }

  subscribe(destination, listener) {
    if (!this.isConnected.apply(this)){
      // connection doesn't exist, connect and subscribe
      return this.connect.apply(this)
        .then(() => {
          return new Promise((resolve, reject) => {

            // TODO: TEMPORARY CODE pause for 2 seconds to allow STOMP to make the connection to rule out race condition
            setTimeout(() => {
              console.debug('performed 2 seconds pause');
              resolve();
            }, 2000);

          });
        })
        .then(performSubscribe.bind(this));
    } else {

      // connection exists, just perform subscribe
      return new Promise((resolve, reject) => {
        resolve();
      }).then(performSubscribe.bind(this));

    }

    function performSubscribe() {
      console.debug('performing subscribe');
      let subId = uuid.v4();
      this.subIdToTopic.set(subId, destination);
      if (!this.destinationToListeners.has(destination)) {
        this.destinationToListeners.set(destination, new HashMap());
        let context = {
          connectionManager: this,
          destination: destination
        };
        let subscription = this.client.subscribe(destination, this.onAllMessage.bind(context));
        this.destinationToSubscription.set(destination, subscription);
      }
      this.destinationToListeners.get(destination).set(subId, listener);
      return subId;
    }
  }

  unsubscribe(subId) {
    let destination = this.subIdToTopic.get(subId);
    this.subIdToTopic.remove(subId);
    let subIds = this.destinationToListeners.get(destination);
    subIds.remove(subId)
    if (subIds.count() == 0) {
      this.destinationToListeners.remove(destination);
      this.destinationToSubscription.get(destination).unsubscribe();
      this.destinationToSubscription.remove(destination);
    }
  }

  send(destination, body) {
    if (!this.isConnected.apply(this)){
      this.connect.apply(this);
    }

    this.client.send(destination, body);
  }
}

export default StompConnectionManager;
