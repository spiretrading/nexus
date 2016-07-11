import pubsub from 'pubsub-js';

function EventBus(){
    // PRIVATE
    
    // PUBLIC
    this.subscribe = (eventName, eventListener) => {
        return pubsub.subscribe(eventName, cloneAndPassToEventListener);

        function cloneAndPassToEventListener(eventName, eventPayload){
            let clonedEventPayload = clone(eventPayload);
            eventListener(eventName, clonedEventPayload);
        }
    }

    this.unsubscribe = (eventListenerId) => {
        pubsub.unsubscribe(eventListenerId);
    }

    this.publish = (eventName, eventPayload) => {
        Object.freeze(eventPayload);
        pubsub.publish(eventName, eventPayload);
    }
}

export default new EventBus();