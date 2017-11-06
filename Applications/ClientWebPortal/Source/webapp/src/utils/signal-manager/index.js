import HashMap from 'hashmap';
import uuid from 'uuid';
import { isFunction } from 'utils/value-validator';

export default class {
  constructor() {
    this.listeners = new HashMap();
    this.subIdCounter = 0;
  }

  addListener(listener) {
    if (!isFunction(listener)) {
      let errorMessage = 'Listener must be a function';
      throw new TypeError(errorMessage);
    }
    this.subId++;
    this.listeners.set(this.subId, listener);
    return this.subId;
  }

  removeListener(subId) {
    if (!this.listeners.has(subId)) {
      let errorMessage = 'Subscription ID does not exist';
      throw new RangeError(errorMessage);
    }
    this.listeners.remove(subId);
  }

  emitSignal(signalType, payload) {
    let listeners = this.listeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](signalType, payload);
    }
  }
}
