import * as React from 'react';
import { SegmentedSpinner } from '../../..';

interface Properties {

  /** A unique identifier for the status message. */
  id: string;

  /** The status of the report. */
  status: ReportStatusIndicator.Status;
}

/** Displays a status indicator for the profit and loss report. */
export function ReportStatusIndicator(props: Properties) {
  if(props.status === ReportStatusIndicator.Status.NONE) {
    return <div/>;
  }
  const icon = (() => {
    if(props.status === ReportStatusIndicator.Status.READY) {
      return (
        <img
          src='resources/account_page/profit_and_loss_page/ready.svg'
          style={STYLE.icon} aria-hidden='true'/>);
    } else if(props.status === ReportStatusIndicator.Status.STALE) {
      return (
        <img
          src='resources/account_page/profit_and_loss_page/stale.svg'
          style={STYLE.icon} aria-hidden='true'/>);
    } else if(props.status === ReportStatusIndicator.Status.IN_PROGRESS) {
      return <SegmentedSpinner color='#333333'/>;
    }
    return null;
  })();
  const text = (() => {
    switch(props.status) {
      case ReportStatusIndicator.Status.IN_PROGRESS:
        return 'Generating report\u2026';
      case ReportStatusIndicator.Status.READY:
        return 'Report ready';
      case ReportStatusIndicator.Status.STALE:
        return 'Report needs update';
    }
  })();
  return (
    <span id={props.id} role='status' style={STYLE.container}>
      {icon}
      {text}
    </span>);
}

export namespace ReportStatusIndicator {

  /** Enumerates the possible states of a report. */
  export enum Status {

    /** The default state, no report has been generated. */
    NONE,

    /** The report is currently being generated. */
    IN_PROGRESS,

    /** The report has been generated and is ready. */
    READY,

    /** The report is out of date and needs to be regenerated. */
    STALE
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  container: {
    display: 'inline-flex',
    alignItems: 'center',
    gap: '4px',
    color: '#333333',
    fontSize: '0.875rem'
  },
  icon: {
    width: '16px',
    height: '16px',
    flexShrink: 0
  }
};
