import React from 'react';
import UpdatableView from 'commons/updatable-view';
import HoverableIcon from 'components/reusables/hoverable-svg-icon';
import imageResLoader from 'utils/image-res-loader';

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
        <HoverableIcon className="menu-btn" srcNotHover="images/icons/burger.png"
                       srcHover="images/icons/burger_hover.png"
                       padding="8px 5px 8px 5px"
                       isButton="true"
                       onClick={this.onMenuClick.bind(this)}/>
        <img className="logo" src={imageResLoader.getResPath("images/top_logo.png")}/>
      </div>
    );
  }
}

export default View;