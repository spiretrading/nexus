import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The effective date. */
  date: Date;

  /** Whether the request has been approved. */
  isApproved: boolean;

  /** The current date. Defaults to today. */
  today?: Date;
}

/** Displays the effective date and status of a change request. */
export class RequestEffectiveDate extends React.Component<Properties> {
  public render(): JSX.Element {
    const status = this.getStatus();
    const prefix = this.getPrefix(status);
    return (
      <div className={css(STYLES.container)}>
        <div aria-hidden className={css(STYLES.calendar)}/>
        <span className={css(STYLES.text)}>{prefix}</span>
        <time className={css(STYLES.text)}
          dateTime={formatISODate(this.props.date)}>
          {formatLabel(this.props.date)}
        </time>
        {status === Status.UPCOMING &&
          <span className={css(STYLES.text)}>
            (in {this.formatTimeToDate()})
          </span>}
      </div>);
  }

  private getToday(): Date {
    return this.props.today !== undefined ? this.props.today : new Date();
  }

  private getStatus(): Status {
    return this.getToday() < this.props.date ? Status.UPCOMING : Status.LAPSED;
  }

  private getPrefix(status: Status): string {
    if(status === Status.UPCOMING && this.props.isApproved) {
      return 'Scheduled';
    } else if(status === Status.LAPSED && this.props.isApproved) {
      return 'Applied';
    }
    return 'Requested';
  }

  private formatTimeToDate(): string {
    const today = this.getToday();
    const diffMs = this.props.date.getTime() - today.getTime();
    const diffDays = Math.ceil(diffMs / (1000 * 60 * 60 * 24));
    if(diffDays >= 14) {
      return `${Math.floor(diffDays / 7)}w`;
    }
    return `${diffDays}d`;
  }
}

enum Status {
  UPCOMING,
  LAPSED
}

function formatISODate(date: Date): string {
  const y = date.getFullYear();
  const m = String(date.getMonth() + 1).padStart(2, '0');
  const d = String(date.getDate()).padStart(2, '0');
  return `${y}-${m}-${d}`;
}

function formatLabel(date: Date): string {
  return date.toLocaleDateString('en-US', {
    month: 'short', day: 'numeric', year: 'numeric'
  });
}

const STYLES = StyleSheet.create({
  container: {
    display: 'inline-flex',
    alignItems: 'center' as 'center',
    gap: '4px'
  },
  calendar: {
    width: '12px',
    height: '12px',
    flexShrink: 0,
    backgroundColor: '#7D7E90',
    WebkitMaskImage: 'url(resources/requests_page/calendar.svg)',
    maskImage: 'url(resources/requests_page/calendar.svg)',
    WebkitMaskSize: 'contain',
    maskSize: 'contain',
    WebkitMaskRepeat: 'no-repeat',
    maskRepeat: 'no-repeat'
  },
  text: {
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    color: '#5D5E6D',
    whiteSpace: 'nowrap' as 'nowrap'
  }
});
