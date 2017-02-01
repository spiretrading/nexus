import View from './view';

class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.notifications = [];
    this.componentModel.isOpen = false;
    this.componentModel.hasEverBeenOpened = false;
    this.view = new View(react, this, this.componentModel);
  }

  componentDidMount() {
    this.onOpenListenerId = EventBus.subscribe(Event.TopNav.OPEN_NOTIFICATION_PANEL, this.onOpen.bind(this));
    this.onCloseListenerId = EventBus.subscribe(Event.TopNav.CLOSE_NOTIFICATION_PANEL, this.onClose.bind(this));
    this.onUpdateListenerId = EventBus.subscribe(Event.TopNav.UPDATE_NOTIFICATION_PANEL, this.onUpdate.bind(this));
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.onOpenListenerId);
    EventBus.unsubscribe(this.onCloseListenerId);
    EventBus.unsubscribe(this.onUpdateListenerId);
  }

  onOpen(eventName, notifications) {
    this.componentModel.notifications = notifications;
    this.componentModel.isOpen = true;
    this.componentModel.hasEverBeenOpened = true;
    this.view.update.apply(this.view, [this.componentModel]);
  }

  onClose() {
    this.componentModel.isOpen = false;
    this.view.update.apply(this.view, [this.componentModel]);
  }

  onUpdate(eventName, notifications) {
    this.componentModel.notifications = notifications;
    this.view.update.apply(this.view, [this.componentModel]);
  }

  itemClicked(itemIndex) {
    EventBus.publish(Event.TopNav.NOTIFICATION_ITEM_SELECTED, itemIndex);
    this.componentModel.isOpen = false;
    this.view.update.apply(this.view, [this.componentModel]);
  }

  setItemClickStatus(itemIndex, isClicked) {
    EventBus.publish(Event.TopNav.NOTIFICATION_ITEM_READ_UPDATED, {
      itemIndex: itemIndex,
      isRead: isClicked
    });
  }

  getView() {
    return this.view;
  }
}

export default Controller;
