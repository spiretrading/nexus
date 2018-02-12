import UpdatableView from 'commons/updatable-view';
import React from 'react';
import {
  AccountModificationRequestType,
  AccountModificationRequestStatus
} from 'spire-client';
import moment from 'moment';

export default class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onPanelClick = this.onPanelClick.bind(this);
  }

  onPanelClick() {
    this.controller.onClick();
  }

  getTitle(accountModificationType) {
    if (accountModificationType == AccountModificationRequestType.ENTITLEMENTS) {
      return 'Entitlements Request';
    } else if (accountModificationType == AccountModificationRequestType.RISK_CONTROLS) {
      return 'Risk Controls Request';
    }
  }

  getDateTimestamp(time) {
    let timestamp = moment.utc(time, 'YYYYMMDDTHHmmss');
    let utcOffset = (new Date().getTimezoneOffset()) * -1;
    timestamp = timestamp.utcOffset(utcOffset);
    return {
      dateTimestamp: timestamp.format('DD/MM/YYYY | HH:mm'),
      datestamp: timestamp.format('DD/MM/YYYY'),
      timestamp: timestamp.format('HH:mm')
    };
  }

  getStatus(status) {
    if (status == AccountModificationRequestStatus.PENDING) {
      return <span className="yellow">Pending</span>;
    } else if (status == AccountModificationRequestStatus.SCHEDULED || status == AccountModificationRequestStatus.GRANTED) {
      return <span className="green">Approved</span>;
    } else if (status == AccountModificationRequestStatus.REJECTED) {
      return <span className="red">Rejected</span>;
    }
  }
}
