import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLSpanElement>, 'children'> {

  /** The status of the report. */
  status: ReportStatusTag.Status;
}

/** Displays whether a report is generating, failed, or ready to view. */
export function ReportStatusTag(props: Properties): JSX.Element {
  const {status, className, ...attributes} = props;
  const label = (() => {
    switch(status) {
      case ReportStatusTag.Status.GENERATING:
        return 'Generating';
      case ReportStatusTag.Status.FAILED:
        return 'Failed';
      case ReportStatusTag.Status.READY:
        return 'Ready';
    }
  })();
  return <span {...attributes}
      className={[css(STYLES.tag,
        status === ReportStatusTag.Status.FAILED && STYLES.failed,
        status === ReportStatusTag.Status.READY && STYLES.ready),
        className].join(' ')}>
    {label}
  </span>;
}

export namespace ReportStatusTag {

  /** The status of a report. */
  export enum Status {

    /** The report is being generated. */
    GENERATING,

    /** Report generation failed. */
    FAILED,

    /** The report is ready to view. */
    READY
  }
}

const STYLES = StyleSheet.create({
  tag: {
    boxSizing: 'border-box',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    flexShrink: 0,
    width: '80px',
    height: '22px',
    borderRadius: '4px',
    fontSize: '0.75rem',
    fontFamily: 'Roboto, system-ui, sans-serif',
    backgroundColor: '#F2F9FF',
    color: '#094A85'
  },
  failed: {
    backgroundColor: '#FFF1F1',
    color: '#941B1B'
  },
  ready: {
    backgroundColor: '#EBFFF2',
    color: '#0C7032'
  }
});
