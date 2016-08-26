import './style.scss';
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
          <div className="info-container">
            <span className="user-name">{this.componentModel.userName}</span>
            <span title="Trader" className={traderClass}></span>
            <span title="Manager" className={managerClass}></span>
            <span title="Admin" className={adminClass}></span>
            <span title="Service" className={serviceClass}></span>
          </div>
        </div>
    );
  }
}

export default View;