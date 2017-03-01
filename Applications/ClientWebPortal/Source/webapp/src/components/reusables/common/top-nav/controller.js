import View from './view';

/** Top nav controller */
class Controller {
  constructor(react) {
    this.componentModel = {};
    this.view = new View(react, this, this.componentModel);
  }

  componentDidMount() {
    this.subMenuUpdatedEventListenerId = EventBus.subscribe(Event.TopNav.SUBMENU_UPDATED, this.onSubMenuUpdated.bind(this));
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.subMenuUpdatedEventListenerId);
  }

  /** @private */
  onSubMenuUpdated(eventName, doesSubMenuExist) {
    if (doesSubMenuExist) {
      this.view.showBottomBorder.apply(this.view);
    } else {
      this.view.hideBottomBorder.apply(this.view);
    }
  }

  getView() {
    return this.view;
  }

  openMenu() {
    EventBus.publish(Event.TopNav.SIDE_MENU_OPENED);
  }
}

export default Controller;
