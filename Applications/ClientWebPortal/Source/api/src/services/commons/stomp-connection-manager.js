import HashMap from 'hashmap';
import uuid from 'uuid';
import webstomp from 'webstomp-client';

/** Spire STOMP connection manager */
class StompConnectionManager {
  constructor(connectionPath) {
    this.connectionUrl = 'ws://' + window.location.host + Config.BACKEND_API_ROOT_URL + connectionPath;
    this.subIdToTopic = new HashMap();
    this.destinationToListeners = new HashMap();      // subId - listener callbacks
    this.destinationToSubscription = new HashMap();
  }

  connect() {
    return new Promise((resolve, reject) => {
      let websocket = new WebSocket(this.connectionUrl);
      this.client = webstomp.over(websocket, {
        debug: false
      });
      this.client.connect({}, () => {
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
    if (!this.isConnected()){
      // connection doesn't exist, connect and subscribe
      return this.connect()
        .then(performSubscribe.bind(this));
    } else {

      // connection exists, just perform subscribe
      return new Promise((resolve, reject) => {
        resolve();
      }).then(performSubscribe.bind(this));

    }

    function performSubscribe() {
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
    let payload = JSON.stringify(body);
    console.debug(payload);

    if (!this.isConnected()){
      this.connect()
        .then(() => {
          this.client.send(payload);
        });
    } else {
      this.client.send(payload);
    }
  }
}

export default StompConnectionManager;
