import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';
import Notification from 'components/reusables/common/notification';
import NotificationDisplayPanel from 'components/reusables/mobile/notification-display-panel';
import SubPagesMenu from 'components/reusables/common/sub-pages-menu';

/** Top nav view */
class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  onMenuClick() {
    this.controller.openMenu();
  }

  render() {
    return (
      <div id="top-nav-container">
        <div className="header">
          <span className="icon-burger" onClick={this.onMenuClick.bind(this)}></span>
          <img className="logo" src={imageResLoader.getResPath("/images/top_logo.png")}/>
          <Notification/>
        </div>
        <div>
          <NotificationDisplayPanel/>
          <SubPagesMenu/>
        </div>
      </div>
    );
  }
}

export default View;
