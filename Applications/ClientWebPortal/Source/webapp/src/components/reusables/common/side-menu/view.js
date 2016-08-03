import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onMenuCloseClick() {
    $('#side-menu-container').stop().fadeOut({
      duration: 60
    });
  }

  /** @private */
  onSignOutClick() {
    this.controller.signOut();
    this.onMenuCloseClick();
  }

  /** @private */
  onMyProfileClick() {
    this.onMenuCloseClick();
    this.controller.navigateToMyProfileAccount();
  }

  /** @private */
  onSearchProfilesClick() {
    this.onMenuCloseClick();
    this.controller.navigateToSearchProfiles();
  }

  initialize() {
    $('#side-menu-container .close-btn').on('mouseenter', () => {
      $('#side-menu-container .close-btn').addClass('close-btn-hover');
    }).on('mouseleave', () => {
      $('#side-menu-container .close-btn').removeClass('close-btn-hover');
    });
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
          <img src={imageResLoader.getResPath("images/top_logo_inverted.png")}/>
        </div>
        <span className="icon-burger close-btn" onClick={this.onMenuCloseClick.bind(this)}></span>
        <div className="menu-item" onClick={this.onMyProfileClick.bind(this)}>
          <span className="icon-my_profile"></span>
          My Profile
        </div>
        {searchProfilesMenuItem}
        <div className="menu-item">
          <span className="icon-reports"></span>
          Reports
        </div>
        <div className="menu-item">
          <span className="icon-portfolio"></span>
          Portfolio
        </div>
        <div className="menu-item" onClick={this.onSignOutClick.bind(this)}>
          <span className="icon-signout"></span>
          Sign Out
        </div>
      </div>
    );
  }
}

export default View;