import HashMap from 'hashmap';
import uuid from 'uuid';

export default class {
  constructor() {
    this.listeners = new HashMap();
  }

  addListener(listener) {
    let subId = uuid.v4();
    this.listeners.set(subId, listener);
    return subId;
  }

  removeListener(subId) {
    this.listeners.remove(subId);
  }

  emitSignal(signalType, payload) {
    let listeners = this.listeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](signalType, payload);
    }
  }
}
