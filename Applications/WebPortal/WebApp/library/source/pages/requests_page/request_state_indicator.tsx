import * as React from 'react';

export enum RequestState {
  SUBMITTED,
  MANAGER_APPROVED,
  APPROVED,
  REJECTED
}

interface Properties {

  /** The state of the request. */
  state: RequestState;
}

/** Displays an icon indicating the state of a request. */
export class RequestStateIndicator extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <img width='16' height='16' alt={getAltText(this.props.state)}
        src={getSrc(this.props.state)}/>);
  }
}

function getSrc(state: RequestState): string {
  switch(state) {
    case RequestState.SUBMITTED:
    case RequestState.MANAGER_APPROVED:
      return 'resources/requests_page/pending.svg';
    case RequestState.APPROVED:
      return 'resources/requests_page/approved.svg';
    case RequestState.REJECTED:
      return 'resources/requests_page/rejected.svg';
  }
}

function getAltText(state: RequestState): string {
  switch(state) {
    case RequestState.SUBMITTED:
      return 'Submitted';
    case RequestState.MANAGER_APPROVED:
      return 'Manager Approved';
    case RequestState.APPROVED:
      return 'Approved';
    case RequestState.REJECTED:
      return 'Rejected';
  }
}
