import View from './view';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(react) {
    this.componentModel = react.props.model || [];
    this.view = new View(react, this, this.componentModel);
  }

  componentDidMount() {
    this.pageTransitionedEventListenerId = EventBus.subscribe(Event.Application.PAGE_TRANSITIONED, this.onPageTransitioned.bind(this));
    this.onPageTransitioned.apply(this, [null, window.location.pathname]);
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.pageTransitionedEventListenerId);
  }

  /** @private */
  onPageTransitioned(eventName, path) {
    if (path.indexOf('/profile') >= 0) {
      this.componentModel = this.getProfilePages(path);
      EventBus.publish(Event.TopNav.SUBMENU_UPDATED, true);
    } else {
      this.componentModel = [];
      EventBus.publish(Event.TopNav.SUBMENU_UPDATED, false);
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
        iconClass: 'icon-account',
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
      },
      {
        name: 'Performance',
        iconClass: 'icon-performance',
        isActive: 'performance' === subPath,
        path: '/profile-performance/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      }
    ];
    return subPages;
  }

  getView() {
    return this.view;
  }

  navigateTo(path) {
    browserHistory.push(path);
  }
}

export default Controller;
