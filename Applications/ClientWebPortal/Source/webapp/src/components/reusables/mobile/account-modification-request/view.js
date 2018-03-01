import './style.scss';
import React from 'react';
import CommonView from 'components/reusables/common/account-modification-request/common-view';

export default class View extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let containerClass = 'account-modification-request-container no-select ';
    containerClass += this.componentModel.className;
    let title = this.getTitle(this.componentModel.request.requestType);
    let account = this.componentModel.request.account.name;
    let requestedBy = this.componentModel.request.submissionAccount.name;
    let requestId = this.componentModel.request.id;
    let dateTimestamp = this.getDateTimestamp(this.componentModel.request.timestamp);
    let status = this.getStatus(this.componentModel.update.status);

    return (
        <div id={this.componentModel.componentId} className={containerClass} onClick={this.onPanelClick}>
          <div className="title">{title}</div>
          <div className="status">{status}</div>
          <div className="id">{requestId}</div>
          <div className="account">account: {account}</div>
          <div className="datestamp">{dateTimestamp.datestamp}</div>
          <div className="timestamp">{dateTimestamp.timestamp}</div>
        </div>
    );
  }
}
