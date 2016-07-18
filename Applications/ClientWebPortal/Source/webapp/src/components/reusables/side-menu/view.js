import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';
import HoverableIcon from 'components/reusables/hoverable-svg-icon';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  openMenu() {
    $('#side-menu-container').stop().fadeIn({
      duration: 60
    });
  }

  onMenuCloseClick() {
    $('#side-menu-container').stop().fadeOut({
      duration: 60
    });
  }

  onSignOutClick() {
    this.controller.signOut();
    this.onMenuCloseClick();
  }

  render() {
    return (
        <div id="side-menu-container">
          <div className="logo">
            <img src={imageResLoader.getResPath("images/top_logo_inverted.png")}/>
          </div>
          <div className="close-btn-container">
            <HoverableIcon className="close-btn" srcNotHover="images/icons/close_white.png"
                           srcHover="images/icons/close_white_hover.png"
                           padding="0"
                           isButton="true"
                           onClick={this.onMenuCloseClick.bind(this)}/>
          </div>
          <div className="menu-item">
            <img src={imageResLoader.getResPath("images/icons/profile.png")}/>
            Profile
          </div>
          <div className="menu-item">
            <img src={imageResLoader.getResPath("images/icons/reports.png")}/>
            Reports
          </div>
          <div className="menu-item">
            <img src={imageResLoader.getResPath("images/icons/portfolio.png")}/>
            Portfolio
          </div>
          <div className="menu-item" onClick={this.onSignOutClick.bind(this)}>
            <img src={imageResLoader.getResPath("images/icons/signout.png")}/>
            Sign Out
          </div>
        </div>
    );
  }
}

export default View;