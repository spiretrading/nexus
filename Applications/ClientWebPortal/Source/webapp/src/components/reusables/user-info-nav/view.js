import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let defaultClass = 'icon icon-';
    let traderClass = defaultClass + 'trader';
    let managerClass = defaultClass + 'manager';
    let adminClass = defaultClass + 'admin';
    let serviceClass = defaultClass + 'service';

    if (this.componentModel.roles != null) {
      if (this.componentModel.roles.isTrader) {
        traderClass += ' active';
      }
      if (this.componentModel.roles.isManager) {
        managerClass += ' active';
      }
      if (this.componentModel.roles.isAdmin) {
        adminClass += ' active';
      }
      if (this.componentModel.roles.isService) {
        serviceClass += ' active';
      }
    }

    return (
        <div className="user-info-nav-container">
          {'<'} Back
          <div className="info-container">
            <span className="user-name">{this.componentModel.userName}</span>
            <span className={traderClass}></span>
            <span className={managerClass}></span>
            <span className={adminClass}></span>
            <span className={serviceClass}></span>
          </div>
        </div>
    );
  }
}

export default View;