import View from './view';
import userService from 'services/user';
import {browserHistory} from 'react-router/es6';

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
    browserHistory.push('profile-account');
  }

  navigateToSearchProfiles() {
    browserHistory.push('searchProfiles');
  }
}

export default Controller;