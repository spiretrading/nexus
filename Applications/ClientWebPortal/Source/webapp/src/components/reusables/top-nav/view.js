import React from 'react';
import UpdatableView from 'commons/updatable-view';
import imageResLoader from 'utils/image-res-loader';
import SubPages from 'components/reusables/top-nav/sub-pages';

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
        <span className="icon-burger" onClick={this.onMenuClick.bind(this)}></span>
        <img className="logo" src={imageResLoader.getResPath("images/top_logo.png")}/>
        <div className="sub-pages-wrapper">
          <SubPages model={this.componentModel.subPages}/>
        </div>
      </div>
    );
  }
}

export default View;