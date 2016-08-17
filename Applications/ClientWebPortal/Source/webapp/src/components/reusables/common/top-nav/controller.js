import View from './view';
import imageResLoader from 'utils/image-res-loader';

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
    if (path.indexOf('/profile') >= 0){
      this.componentModel.subPages = this.getProfilePages(path);
    }
    this.view.update(this.componentModel);
  }

  /** @private */
  getProfilePages(path) {
    let dashIndex = path.indexOf('-');
    let subPath = path.substring(dashIndex + 1);
    let subPages = [
      {
        name: 'Account',
        iconClass: 'icon-my_profile',
        isActive: 'account' === subPath
      },
      {
        name: 'Risk Controls',
        iconClass: 'icon-risk_manager',
        isActive: 'riskControls' === subPath
      },
      {
        name: 'Entitlements',
        iconClass: 'icon-entitlements',
        isActive: 'entitlements' === subPath
      },
      {
        name: 'Compliance',
        iconClass: 'icon-compliance',
        isActive: 'compliance' === subPath
      }
    ];
    return subPages;
  }

  componentDidMount() {
    this.pageTransitionedEventListenerId = EventBus.subscribe(Event.Application.PAGE_TRANSITIONED, this.onPageTransitioned.bind(this));
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.pageTransitionedEventListenerId);
  }

  getView() {
    return this.view;
  }

  openMenu() {
    EventBus.publish(Event.TopNav.MENU_OPENED);
  }
}

export default Controller;