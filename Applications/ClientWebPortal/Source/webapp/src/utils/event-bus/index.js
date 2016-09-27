import pubsub from 'pubsub-js';

/** Web app internal event bus */
class EventBus {
  subscribe(eventName, eventListener) {
    return pubsub.subscribe(eventName, cloneAndPassToEventListener);

    function cloneAndPassToEventListener(eventName, eventPayload) {
      let clonedEventPayload;
      if (eventPayload != null) {
        clonedEventPayload = cloneObject(eventPayload);
      }
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
