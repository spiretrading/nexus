import React, {PropTypes, Component} from 'react';
import TopNav from 'components/reusables/top-nav';
import SideMenu from 'components/reusables/side-menu';
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

  componentDidUpdate() {
    this.saveLastVisitedPath();
    this.publishPageTransitioned();
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
