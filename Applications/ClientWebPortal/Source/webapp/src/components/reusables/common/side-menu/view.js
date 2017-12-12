import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.closeMenu = this.closeMenu.bind(this);
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
  onModificationRequestsClick() {
    this.closeMenu();
    this.controller.navigateToModificationRequestHistory();
  }

  /** @private */
  onMyProfileClick() {
    this.closeMenu();
    this.controller.navigateToProfileAccount();
  }

  /** @private */
  onAccountsClick() {
    this.closeMenu();
    this.controller.navigateToAccounts();
  }

  /** @private */
  onPortfolioClick() {
    this.closeMenu();
    this.controller.navigateToPortfolio();
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
    let searchProfilesMenuItem, portfolioMenuItem;
    if (this.componentModel.isAdmin || this.componentModel.isManager) {
      searchProfilesMenuItem =
        <div className="menu-item" onClick={this.onAccountsClick.bind(this)}>
          <span className="icon-search"></span>
          Accounts
        </div>

      portfolioMenuItem =
        <div className="menu-item" onClick={this.onPortfolioClick.bind(this)}>
          <span className="icon-portfolio"></span>
          Portfolio
        </div>
    }

    return (
      <div id="side-menu-container">
        <div className="logo">
          <img src={imageResLoader.getResPath("/images/top_logo_inverted.png")}/>
        </div>
        <span className="icon-burger close-btn" onClick={this.closeMenu.bind(this)}></span>
        <div className="menu-item" onClick={this.onMyProfileClick.bind(this)}>
          <span className="icon-profile"></span>
          My Profile
        </div>
        {searchProfilesMenuItem}
        {portfolioMenuItem}
        <div className="menu-item" onClick={this.onModificationRequestsClick.bind(this)}>
          <span className="icon-signout"></span>
          Requests
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
