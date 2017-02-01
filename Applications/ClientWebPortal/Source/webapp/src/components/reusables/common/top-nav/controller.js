import View from './view';

/** Top nav controller */
class Controller {
  constructor(react) {
    this.componentModel = {};
    this.view = new View(react, this, this.componentModel);
  }

  getView() {
    return this.view;
  }

  openMenu() {
    EventBus.publish(Event.TopNav.SIDE_MENU_OPENED);
  }
}

export default Controller;
