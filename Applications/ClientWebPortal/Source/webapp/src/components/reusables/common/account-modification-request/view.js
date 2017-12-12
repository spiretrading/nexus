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
  }

  render() {
    let containerClass = 'account-modification-request-container ';
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
    timestamp = timestamp.format('DD/MM/YYYY | HH:mm');

    let status = this.componentModel.update.status;
    if (status == AccountModificationRequestStatus.PENDING) {
      status = <span>Pending</span>;
    } else if (status == AccountModificationRequestStatus.SCHEDULED || status == AccountModificationRequestStatus.GRANTED) {
      status = <span className="green">Approved</span>;
    } else if (status == AccountModificationRequestStatus.REJECTED) {
      status = <span className="red">Rejected</span>;
    }

    return (
        <div id={this.componentModel.componentId} className={containerClass}>
          <div className="left-panel">
            <div className="title">
              {title}
            </div>
            <div className="account">
              <div className="legend">Account</div>
              {account}
            </div>
            <div className="requester">
              <div className="legend">Requested by</div>
              {requestedBy}
            </div>
            <div className="request-id">
              <div className="legend">Requested ID</div>
              {requestId}
            </div>
          </div>
          <div className="right-panel">
            <div className="timestamp">{timestamp}</div>
            <div className="status">
              <div className="legend">Status</div>
              {status}
            </div>
          </div>
        </div>
    );
  }
}

export default View;
