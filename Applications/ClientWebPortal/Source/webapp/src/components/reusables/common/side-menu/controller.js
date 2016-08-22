import View from './view';
import userService from 'services/user';
import {browserHistory} from 'react-router/es6';
import store from 'store';
import LocalstorageKey from 'commons/localstorage-keys';

class Controller {
  constructor(react) {
    this.componentModel = {
      isAdmin: false,
      isManager: false
    };
    this.view = new View(react, this, this.componentModel);
  }

  /** @private */
  onMenuOpened() {
    this.view.openMenu();
  }

  /** @private */
  onSignedIn() {
    this.componentModel = {
      isAdmin: userService.isAdmin(),
      isManager: userService.isManager()
    };
    this.view.update(this.componentModel);
  }

  /** @private */
  saveLastVisitedPath(path) {
    store.set(LocalstorageKey.LAST_VISITED_PATH, path);
  }

  getView() {
    return this.view;
  }

  componentDidMount() {
    this.view.initialize();
    this.menuOpenedEventListenerId = EventBus.subscribe(Event.TopNav.MENU_OPENED, this.onMenuOpened.bind(this));
    this.signedInEventListenerId = EventBus.subscribe(Event.Application.SIGNED_IN, this.onSignedIn.bind(this));
  }

  componentWillUnmount() {
    this.view.dispose();
    EventBus.unsubscribe(this.menuOpenedEventListenerId);
    EventBus.unsubscribe(this.signedInEventListenerId);
  }

  signOut() {
    userService.signOut();
    browserHistory.push('/');
  }

  navigateToMyProfileAccount() {
    let path = 'profile-account';
    browserHistory.push(path, userService.getDirectoryEntry());
    this.saveLastVisitedPath(path);
  }

  navigateToSearchProfiles() {
    let path = 'searchProfiles';
    browserHistory.push(path);
    this.saveLastVisitedPath(path);
  }
}

export default Controller;
