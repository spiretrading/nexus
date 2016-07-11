import pubsub from 'pubsub-js';

class EventBus {
  subscribe(eventName, eventListener) {
    return pubsub.subscribe(eventName, cloneAndPassToEventListener);

    function cloneAndPassToEventListener(eventName, eventPayload) {
      let clonedEventPayload = clone(eventPayload);
      eventListener(eventName, clonedEventPayload);
    }
  }

  unsubscribe(eventListenerId) {
    pubsub.unsubscribe(eventListenerId);
  }

  publish(eventName, eventPayload) {
    pubsub.publish(eventName, eventPayload);
  }
}

export default new EventBus();