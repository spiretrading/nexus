import React, {PropTypes, Component} from 'react';
import TopNav from 'components/reusables/common/top-nav';
import SideMenu from 'components/reusables/common/side-menu';
import store from 'store';
import LocalstorageKey from 'commons/localstorage-keys';
import deviceDetector from 'utils/device-detector';
import './style.scss';

class App extends Component {
  constructor(props) {
    super(props);
  }

  /** @private */
  saveLastVisitedPath() {
    let currentLocation = this.props.location.pathname;
    if (currentLocation != '/') {
      store.set(LocalstorageKey.LAST_VISITED_PATH, currentLocation);
    }
  }

  /** @private */
  publishPageTransitioned() {
    let pathName = window.location.pathname;
    EventBus.publish(Event.Application.PAGE_TRANSITIONED, pathName);
  }

  /** @private */
  hideIfHome() {
    let pathName = window.location.pathname;
    if (pathName === '/'){
      $('#top-nav-wrapper').css('display', 'none');
      $('#top-nav-filler').css('display', 'none');
      $('#side-menu-wrapper').css('display', 'none');
    } else {
      $('#top-nav-wrapper').css('display', 'inherit');
      $('#top-nav-filler').css('display', 'inherit');
      $('#side-menu-wrapper').css('display', 'inherit');
    }
  }

  componentDidMount() {
    this.hideIfHome();
  }

  componentDidUpdate() {
    this.saveLastVisitedPath();
    this.publishPageTransitioned();
    this.hideIfHome();
  }

  render() {
    let className = 'viewport';
    if (deviceDetector.isMobile()) {
      className += " mobile";
    }

    return (
      <main className={className}>
        <div id="top-nav-wrapper">
          <TopNav />
        </div>
        <div id="top-nav-filler"></div>
        <div id="side-menu-wrapper">
          <SideMenu />
        </div>
        <div id="site-content-container">
          {this.props.children}
        </div>
      </main>
    );
  }
}

export default App;
