import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  closeMenu() {
    $('#side-menu-container').stop().fadeOut({
      duration: 60
    });
    this.controller.publishSideMenuClosed();
  }

  /** @private */
  onSignOutClick() {
    this.closeMenu();
    this.controller.signOut();
  }

  /** @private */
  onMyProfileClick() {
    this.closeMenu();
    this.controller.navigateToProfileAccount.apply(this.controller);
  }

  /** @private */
  onSearchProfilesClick() {
    this.closeMenu();
    this.controller.navigateToSearchProfiles.apply(this.controller);
  }

  initialize() {
    $('#side-menu-container .close-btn').on('mouseenter', () => {
      $('#side-menu-container .close-btn').addClass('close-btn-hover');
    }).on('mouseleave', () => {
      $('#side-menu-container .close-btn').removeClass('close-btn-hover');
    });

    let sideMenu = document.getElementById('side-menu-container');
    sideMenu.addEventListener('touchcancel', function(e) {
      e.preventDefault();
    }, false);
    sideMenu.addEventListener('touchmove', function(e) {
      e.preventDefault();
    }, false);
  }

  dispose() {
    $('#side-menu-container .close-btn').off();
  }

  openMenu() {
    $('#side-menu-container').stop().fadeIn({
      duration: Config.FADE_DURATION,
      complete: () => {
        $('#side-menu-container .close-btn').mouseover();
      }
    });
  }

  render() {
    let searchProfilesMenuItem;
    if (this.componentModel.isAdmin || this.componentModel.isManager) {
      searchProfilesMenuItem = <div className="menu-item" onClick={this.onSearchProfilesClick.bind(this)}>
        <span className="icon-search"></span>
        Search
      </div>
    }

    return (
      <div id="side-menu-container">
        <div className="logo">
          <img src={imageResLoader.getResPath("/images/top_logo_inverted.png")}/>
        </div>
        <span className="icon-burger close-btn" onClick={this.closeMenu.bind(this)}></span>
        <div className="menu-item" onClick={this.onMyProfileClick.bind(this)}>
          <span className="icon-my_profile"></span>
          My Profile
        </div>
        {searchProfilesMenuItem}
        <div className="menu-item" onClick={this.onSignOutClick.bind(this)}>
          <span className="icon-signout"></span>
          Sign Out
        </div>
      </div>
    );
  }
}

export default View;
