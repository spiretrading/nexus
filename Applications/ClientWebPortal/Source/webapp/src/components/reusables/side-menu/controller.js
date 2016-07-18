import View from './view';
import userService from 'services/user';
import {browserHistory} from 'react-router/es6';

/** Skeleton component controller */
class Controller {
  constructor(react) {
    this.componentModel = {};
    this.view = new View(react, this, cloneObject(this.componentModel));
  }

  /** @private */
  onMenuOpened() {
    this.view.openMenu();
  }

  getView() {
    return this.view;
  }

  componentDidMount() {
    this.menuOpenedEventListenerId = EventBus.subscribe(Event.TopNav.MENU_OPENED, this.onMenuOpened.bind(this));
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.menuOpenedEventListenerId);
  }

  signOut() {
    userService.signOut();
    browserHistory.push('/');
  }
}

export default Controller;