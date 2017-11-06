import HashMap from 'hashmap';
import uuid from 'uuid';
import valueValidator from 'utils/value-validator';

export default class {
  constructor() {
    this.listeners = new HashMap();
  }

  addListener(listener) {
    if (listener == null) {
      let errorMessage = 'Listener function cannot be null';
      throw new RangeError(errorMessage);
    } else if (!valueValidator.isFunction(listener)) {
      let errorMessage = 'Listener must be a function';
      throw new RangeError(errorMessage);
    }

    let subId = uuid.v4();
    this.listeners.set(subId, listener);
    return subId;
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
