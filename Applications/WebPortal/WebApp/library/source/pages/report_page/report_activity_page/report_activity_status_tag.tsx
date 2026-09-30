import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLSpanElement>, 'children'> {

  /** The activity status of the report. */
  status: ReportActivityStatusTag.Status;
}

/** Displays whether a report is being generated or its generation failed. */
export function ReportActivityStatusTag(props: Properties): JSX.Element {
  const {status, className, ...attributes} = props;
  const label = (() => {
    switch(status) {
      case ReportActivityStatusTag.Status.GENERATING:
        return 'Generating';
      case ReportActivityStatusTag.Status.FAILED:
        return 'Failed';
    }
  })();
  return <span {...attributes}
      className={[css(STYLES.tag,
        status === ReportActivityStatusTag.Status.FAILED && STYLES.failed),
        className].join(' ')}>
    {label}
  </span>;
}

export namespace ReportActivityStatusTag {

  /** The activity status of a report. */
  export enum Status {

    /** The report is being generated. */
    GENERATING,

    /** Report generation failed. */
    FAILED
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
  }
});
