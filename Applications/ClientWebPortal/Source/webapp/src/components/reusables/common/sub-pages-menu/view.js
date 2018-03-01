import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import UserInfoNav from 'components/reusables/common/user-info-nav';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onMenuClick(event) {
    let path = $(event.currentTarget).attr('data-path');
    this.controller.navigateTo(path);
  }

  render() {
    let className = 'sub-pages-container';
    let userName;
    if (deviceDetector.isMobile()){
      className += ' mobile';
    }

    if (this.componentModel.subpages.length == 0) {
      className += ' empty';
      $('#top-nav-filler').css('height', '75px');
    } else {
      if (deviceDetector.isMobile()) {
        $('#top-nav-filler').css('height', '165px');
      } else {
        $('#top-nav-filler').css('height', '135px');
      }      

      if (deviceDetector.isMobile() && this.componentModel.userInfoModel != null) {
        userName =  <div className="user-name-wrapper">
                      {this.componentModel.userInfoModel.userName}
                    </div>
      }
    }

    let menuItems = [];
    let subpages = this.componentModel.subpages;
    for (let i=0; i<subpages.length; i++) {
      let className = 'menu-item';
      if (this.componentModel.subpages[i].isActive){
        className += ' active';
      }
      menuItems.push(
        <div className={className} key={i} data-path={subpages[i].path} onClick={this.onMenuClick.bind(this)}>
          <span className={subpages[i].iconClass}></span>
          <div>{subpages[i].name}</div>
        </div>
      );
    }

    let userInfo;
    if (this.componentModel.userInfoModel != null) {
      userInfo =  <div className="user-info-nav-wrapper">
                    <UserInfoNav model={this.componentModel.userInfoModel}/>
                  </div>;
    }

    return (
        <div id="sub-pages-container" className={className}>
          <div className="menu-wrapper">
            {menuItems}
            {userInfo}
          </div>
          {userName}
        </div>
    );
  }
}

export default View;
