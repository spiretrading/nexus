import View from './view';

/** Top nav controller */
class Controller {
  constructor(react) {
    this.componentModel = {
      subPages: []
    };
    this.view = new View(react, this, this.componentModel);
  }

  /** @private */
  onPageTransitioned(eventName, path) {
    if (path.indexOf('/profile') >= 0) {
      this.componentModel.subPages = this.getProfilePages(path);
    } else if (path.indexOf('/searchProfiles') >= 0) {
      this.componentModel.subPages = [];
    }
    this.view.update(this.componentModel);
  }

  /** @private */
  getProfilePages(path) {
    let dashIndex = path.indexOf('-');
    let subPath = path.substring(dashIndex + 1).split('/')[0];
    let params = window.location.pathname.split('/');
    let directoryEntry = {
      type: params[params.length - 3],
      id: params[params.length - 2],
      name: params[params.length - 1]
    };
    let subPages = [
      {
        name: 'Account',
        iconClass: 'icon-my_profile',
        isActive: 'account' === subPath,
        path: '/profile-account/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      },
      {
        name: 'Risk Controls',
        iconClass: 'icon-risk_manager',
        isActive: 'riskControls' === subPath,
        path: '/profile-riskControls/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      },
      {
        name: 'Entitlements',
        iconClass: 'icon-entitlements',
        isActive: 'entitlements' === subPath,
        path: '/profile-entitlements/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      },
      {
        name: 'Compliance',
        iconClass: 'icon-compliance',
        isActive: 'compliance' === subPath,
        path: '/profile-compliance/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      }
    ];
    return subPages;
  }

  componentDidMount() {
    this.pageTransitionedEventListenerId = EventBus.subscribe(Event.Application.PAGE_TRANSITIONED, this.onPageTransitioned.bind(this));
    this.onPageTransitioned.apply(this, [null, window.location.pathname]);
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.pageTransitionedEventListenerId);
  }

  getView() {
    return this.view;
  }

  openMenu() {
    EventBus.publish(Event.TopNav.SIDE_MENU_OPENED);
  }
}

export default Controller;
