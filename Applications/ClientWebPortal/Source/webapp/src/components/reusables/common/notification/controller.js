import View from './view';
import NotificationRepo from './notification-repo';
import NotificationType from './notification-type';

class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.notifications = [];
    this.componentModel.hasNewUnchecked = false;
    this.componentModel.isOpen = false;
    this.notificationRepo = new NotificationRepo();
    this.view = new View(react, this, this.componentModel);
  }

  componentDidMount() {
    this.onReportLoadedListenerId = EventBus.subscribe(Event.Application.REPORT_LOADED, this.onReportLoaded.bind(this));
    this.onItemSelectedListenerId = EventBus.subscribe(Event.TopNav.NOTIFICATION_ITEM_SELECTED, this.onItemSelected.bind(this));
    this.onItemUpdatedListenerId = EventBus.subscribe(Event.TopNav.NOTIFICATION_ITEM_READ_UPDATED, this.onItemReadUpdated.bind(this));
    this.view.initialize.apply(this.view);
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.onReportLoadedListenerId);
    EventBus.unsubscribe(this.onItemSelectedListenerId);
    EventBus.unsubscribe(this.onItemUpdatedListenerId);
  }

  /** @private */
  onItemSelected(eventName, itemIndex) {
    this.itemClicked.apply(this, [itemIndex]);
  }

  /** @private */
  onItemReadUpdated(eventName, payload) {
    let itemIndex = payload.itemIndex;
    let isRead = payload.isRead;
    this.notificationRepo.setItemClicked.apply(this.notificationRepo, [itemIndex, isRead]);
    this.componentModel.notifications = this.notificationRepo.getAll.apply(this.notificationRepo);
    this.componentModel.isOpen = true;
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
    EventBus.publish(Event.TopNav.UPDATE_NOTIFICATION_PANEL, this.notificationRepo.getAll.apply(this.notificationRepo));
  }

  /** @private */
  onReportLoaded(eventName, report) {
    let type = NotificationType.REPORT;
    let title = report.type + ' Completed';
    let description = '';

    for (let parameterName in report.parameters) {
      description += parameterName + ': ' + report.parameters[parameterName] + ', ';
    }

    if (description.length > 0) {
      description = description.substring(0, description.length - 2);
    }

    this.notificationRepo.add(
      type,
      title,
      description
    );

    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
    EventBus.publish(Event.TopNav.UPDATE_NOTIFICATION_PANEL, this.notificationRepo.getAll.apply(this.notificationRepo));
  }

  getView() {
    return this.view;
  }

  closePopup() {
    this.componentModel.isOpen = false;
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
  }

  openPopup() {
    this.componentModel.notifications = this.notificationRepo.getAll.apply(this.notificationRepo);
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.componentModel.isOpen = true;
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
  }

  itemClicked(itemIndex) {
    this.notificationRepo.setItemClicked.apply(this.notificationRepo, [itemIndex, true]);
    this.componentModel.notifications = this.notificationRepo.getAll.apply(this.notificationRepo);
    this.componentModel.isOpen = false;
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
  }

  setItemClickStatus(itemIndex, isClicked) {
    this.notificationRepo.setItemClicked.apply(this.notificationRepo, [itemIndex, isClicked]);
    this.componentModel.notifications = this.notificationRepo.getAll.apply(this.notificationRepo);
    this.componentModel.numUnchecked = this.notificationRepo.getNumUnchecked.apply(this.notificationRepo);
    this.view.update.apply(this.view, [this.componentModel]);
  }

  openPanel() {
    EventBus.publish(Event.TopNav.OPEN_NOTIFICATION_PANEL, this.notificationRepo.getAll.apply(this.notificationRepo));
    this.componentModel.isOpen = true;
    this.view.update.apply(this.view, [this.componentModel]);
  }

  closePanel() {
    EventBus.publish(Event.TopNav.CLOSE_NOTIFICATION_PANEL);
    this.componentModel.isOpen = false;
    this.view.update.apply(this.view, [this.componentModel]);
  }
}

export default Controller;
