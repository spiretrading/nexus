import View from './view';
import {browserHistory} from 'react-router';
import {AdministrationClient} from 'spire-client';

class Controller {
  constructor(react) {
    this.componentModel = {};
    this.componentModel.subpages = react.props.model || [];
    this.view = new View(react, this, this.componentModel);
    this.contextDirectoryEntry = react.props.contextDirectoryEntry;
    this.adminClient = new AdministrationClient();

    this.onPageTransitioned = this.onPageTransitioned.bind(this);
    this.getTraderProfilePages = this.getTraderProfilePages.bind(this);
    this.getGroupProfilePages = this.getGroupProfilePages.bind(this);
    this.navigateTo = this.navigateTo.bind(this);
    this.onProfileViewingContextLoaded = this.onProfileViewingContextLoaded.bind(this);
  }

  componentDidMount() {
    this.pageTransitionedEventListenerId = EventBus.subscribe(Event.Application.PAGE_TRANSITIONED, this.onPageTransitioned.bind(this));
    this.profileViewingContextLoadedListenerId = EventBus.subscribe(
      Event.Profile.VIEWING_CONTEXT_LOADED,
      this.onProfileViewingContextLoaded
    );
    this.onPageTransitioned(null, window.location.pathname);
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.pageTransitionedEventListenerId);
    EventBus.unsubscribe(this.profileViewingContextLoadedListenerId);
  }

  /** @private */
  onPageTransitioned(eventName, path) {
    if (path.indexOf('/profile') >= 0) {
      this.componentModel.subpages = this.getTraderProfilePages(path);
      this.view.update(this.componentModel);
      EventBus.publish(Event.TopNav.SUBMENU_UPDATED, true);
    } else if (path.indexOf('/group-profile') >= 0) {
      this.componentModel.subpages = this.getGroupProfilePages(path);
      this.view.update(this.componentModel);
      EventBus.publish(Event.TopNav.SUBMENU_UPDATED, true);
    } else {
      this.componentModel.subpages = [];
      this.componentModel.userInfoModel = null;
      this.view.update(this.componentModel);
      EventBus.publish(Event.TopNav.SUBMENU_UPDATED, false);
    }
  }

  /** @private */
  onProfileViewingContextLoaded(eventName, payload) {
    this.componentModel.userInfoModel = {
      userName: payload.directoryEntry.name,
      roles: payload.roles
    };
    this.view.update(this.componentModel);
  }

  /** @private */
  getTraderProfilePages(path) {
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
        iconClass: 'icon-risk-controls',
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
        name: 'Profit & Loss',
        iconClass: 'icon-profit-loss',
        isActive: 'performance' === subPath,
        path: '/profile-performance/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      }
    ];
    return subPages;
  }

  /** @private */
  getGroupProfilePages(path) {
    let dashIndex = path.lastIndexOf('-');
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
        iconClass: 'icon-group',
        isActive: 'account' === subPath,
        path: '/group-profile-account/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      },
      {
        name: 'Compliance',
        iconClass: 'icon-compliance',
        isActive: 'compliance' === subPath,
        path: '/group-profile-compliance/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
      },
      {
        name: 'Profit & Loss',
        iconClass: 'icon-profit-loss',
        isActive: 'performance' === subPath,
        path: '/group-profile-performance/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name
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
