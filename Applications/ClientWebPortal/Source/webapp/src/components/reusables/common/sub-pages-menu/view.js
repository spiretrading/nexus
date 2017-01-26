import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onMenuClick(event) {
    let path = $(event.currentTarget).attr('data-path');
    this.controller.navigateTo.apply(this.controller, [path]);
  }

  render() {
    let className = 'sub-pages-container';
    if (deviceDetector.isMobile()){
      className += ' mobile';
    }

    if (this.componentModel.length == 0) {
      className += ' empty';
      $('#top-nav-filler').css('height', '75px');
    } else {
      $('#top-nav-filler').css('height', '125px');
    }

    let menuItems = [];
    for (let i=0; i<this.componentModel.length; i++) {
      let className = 'menu-item';
      if (this.componentModel[i].isActive){
        className += ' active';
      }
      menuItems.push(
        <div className={className} key={i} data-path={this.componentModel[i].path} onClick={this.onMenuClick.bind(this)}>
          <span className={this.componentModel[i].iconClass}></span>
          <div>{this.componentModel[i].name}</div>
        </div>
      );
    }

    return (
        <div id="sub-pages-container" className={className}>
          {menuItems}
        </div>
    );
  }
}

export default View;
