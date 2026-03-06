import * as Nexus from 'nexus';
import * as React from 'react';

type Status = Nexus.AccountModificationRequest.Status;
const Status = Nexus.AccountModificationRequest.Status;

interface Properties {

  /** The state of the request. */
  state: Status;
}

/** Displays an icon indicating the state of a request. */
export function RequestStateIndicator(props: Properties) {
  return (
    <img width='16' height='16' alt={getAltText(props.state)}
      src={getSrc(props.state)}/>);
}

function getSrc(state: Status): string {
  switch(state) {
    case Status.PENDING:
    case Status.REVIEWED:
    case Status.SCHEDULED:
      return 'resources/requests_page/pending.svg';
    case Status.GRANTED:
      return 'resources/requests_page/approved.svg';
    case Status.REJECTED:
      return 'resources/requests_page/rejected.svg';
    default:
      return '';
  }
}

function getAltText(state: Status): string {
  switch(state) {
    case Status.PENDING:
      return 'Submitted';
    case Status.REVIEWED:
      return 'Manager Approved';
    case Status.SCHEDULED:
      return 'Scheduled';
    case Status.GRANTED:
      return 'Approved';
    case Status.REJECTED:
      return 'Rejected';
    default:
      return '';
  }
}
