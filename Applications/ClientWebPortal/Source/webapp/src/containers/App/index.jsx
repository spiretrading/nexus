import React, {PropTypes, Component} from 'react';
import TopNav from 'components/reusables/common/top-nav';
import SideMenu from 'components/reusables/common/side-menu';
import deviceDetector from 'utils/device-detector';
import {DirectoryEntry} from 'spire-client';
import './style.scss';

class App extends Component {
  constructor(props) {
    super(props);
    if (props.params.type != null && props.params.id != null && props.params.name != null) {
      this.contextDirectoryEntry = DirectoryEntry.fromData({
        id: Number(props.params.id),
        type: Number(props.params.type),
        name: props.params.name
      });
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

  /** @private */
  onSideMenuOpened() {
    if (deviceDetector.isMobile()) {
      $('#menu-grey-screen').fadeIn(Config.FADE_DURATION);
    }
  }

  /** @private */
  onSideMenuClosed() {
    if (deviceDetector.isMobile()) {
      $('#menu-grey-screen').fadeOut(Config.FADE_DURATION);
    }
  }

  /** @private */
  isFullScreen() {
    // initial home login screen
    if (location.pathname == '/') {
      return true;
    }

    return false;
  }

  /** @private */
  onSubMenuUpdated(eventName, doesSubMenuExist) {
    let height;
    if (doesSubMenuExist) {
      height = 'calc(100vh - 125px)';
    } else if (this.isFullScreen()) {
      height = 'calc(100vh)';
    } else {
      height = 'calc(100vh - 75px)';
    }

    if (!deviceDetector.isMobile()){
      $('#site-content-container').slimScroll({
        height: height,
        opacity: '0.4'
      });
    }
  }

  componentDidMount() {
    this.hideIfHome();
    EventBus.subscribe(Event.TopNav.SIDE_MENU_OPENED, this.onSideMenuOpened.bind(this));
    EventBus.subscribe(Event.TopNav.SIDE_MENU_CLOSED, this.onSideMenuClosed.bind(this));
    EventBus.subscribe(Event.TopNav.SUBMENU_UPDATED, this.onSubMenuUpdated.bind(this));

    let greyScreen = document.getElementById('menu-grey-screen');
    greyScreen.addEventListener('touchcancel', function(e) {
      e.preventDefault();
    }, false);
    greyScreen.addEventListener('touchmove', function(e) {
      e.preventDefault();
    }, false);

    greyScreen.addEventListener('click', function(e) {
      EventBus.publish(Event.TopNav.CLOSE_SIDE_MENU);
    }, false);

    EventBus.publish(Event.Application.RENDERED);
    this.publishPageTransitioned();
  }

  componentDidUpdate() {
    this.publishPageTransitioned();
    this.hideIfHome();
  }

  render() {
    let className = 'viewport';
    if (deviceDetector.isMobile()) {
      className += " mobile";
    }

    console.debug('app container render()');
    console.debug(this.contextDirectoryEntry);

    return (
      <main className={className}>
        <div id="top-nav-wrapper">
          <TopNav contextDirectoryEntry={this.contextDirectoryEntry} />
        </div>
        <div id="top-nav-filler"></div>
        <div id="side-menu-wrapper">
          <SideMenu />
          <div id="menu-grey-screen"></div>
        </div>
        <div id="site-content-container">
          {this.props.children}
        </div>
      </main>
    );
  }
}

export default App;
