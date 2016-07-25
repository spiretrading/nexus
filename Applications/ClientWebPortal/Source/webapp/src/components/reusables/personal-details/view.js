import React from 'react';
import UpdatableView from 'commons/updatable-view';
import moment from 'moment';

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

    let registrationDate = moment(this.componentModel.registrationDate, moment.ISO_8601).toDate().toLocaleDateString();

    let labels;
    if (this.componentModel.showLabels) {
      labels =
        <div className="labels-container">
          <div className="personal-detail-row">Full Name</div>
          <div className="personal-detail-row">Username</div>
          <div className="personal-detail-row">Role(s)</div>
          <div className="personal-detail-row">Registration Date</div>
          <div className="personal-detail-row">Account Number</div>
          <div className="personal-detail-row">E-mail Address</div>
          <div className="personal-detail-row">Address</div>
          <div className="personal-detail-row">City</div>
          <div className="personal-detail-row">Province/State</div>
          <div className="personal-detail-row">Postal Code</div>
          <div className="personal-detail-row">Country</div>
        </div>
    }

    return (
        <div className="personal-details-container">
          {labels}
          <div className="details-container">
            <div className="personal-detail-row">{this.componentModel.name}</div>
            <div className="personal-detail-row">{this.componentModel.userName}</div>
            <div className="personal-detail-row">
              <span className={traderClass}></span>
              <span className={managerClass}></span>
              <span className={adminClass}></span>
              <span className={serviceClass}></span>
            </div>
            <div className="personal-detail-row">{registrationDate}</div>
            <div className="personal-detail-row">{this.componentModel.accountNumber}</div>
            <div className="personal-detail-row">{this.componentModel.email}</div>
            <div className="personal-detail-row">{this.componentModel.address}</div>
            <div className="personal-detail-row">{this.componentModel.city}</div>
            <div className="personal-detail-row">{this.componentModel.province}</div>
            <div className="personal-detail-row">{this.componentModel.postalCode}</div>
            <div className="personal-detail-row">{this.componentModel.country}</div>
          </div>
        </div>
    );
  }
}

export default View;