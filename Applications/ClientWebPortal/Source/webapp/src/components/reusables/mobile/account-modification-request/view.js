import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import {
  AccountModificationRequestType,
  AccountModificationRequestStatus
} from 'spire-client';
import moment from 'moment';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onPanelClick = this.onPanelClick.bind(this);
  }

  /** @private */
  onPanelClick() {
    this.controller.onClick();
  }

  render() {
    let containerClass = 'account-modification-request-container no-select ';
    containerClass += this.componentModel.className;

    let title;
    if (this.componentModel.request.requestType == AccountModificationRequestType.ENTITLEMENTS) {
      title = 'Entitlements Request';
    }

    let account = this.componentModel.request.account.name;
    let requestedBy = this.componentModel.request.submissionAccount.name;
    let requestId = this.componentModel.request.id;
    let timestamp = moment.utc(this.componentModel.request.timestamp, 'YYYYMMDDTHHmmss');
    let utcOffset = (new Date().getTimezoneOffset()) * -1;
    timestamp = timestamp.utcOffset(utcOffset);
    let datestamp = timestamp.format('DD/MM/YYYY');
    timestamp = timestamp.format('HH:mm');

    let status = this.componentModel.update.status;
    if (status == AccountModificationRequestStatus.PENDING) {
      status = <span className="yellow">Pending</span>;
    } else if (status == AccountModificationRequestStatus.SCHEDULED || status == AccountModificationRequestStatus.GRANTED) {
      status = <span className="green">Approved</span>;
    } else if (status == AccountModificationRequestStatus.REJECTED) {
      status = <span className="red">Rejected</span>;
    }

    return (
        <div id={this.componentModel.componentId} className={containerClass} onClick={this.onPanelClick}>
          <div className="title">{title}</div>
          <div className="status">{status}</div>
          <div className="id">{requestId}</div>
          <div className="account">account: {account}</div>
          <div className="datestamp">{datestamp}</div>
          <div className="timestamp">{timestamp}</div>
        </div>
    );
  }
}

export default View;
