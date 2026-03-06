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
export function RequestEffectiveDate(props: Properties) {
  const today = props.today !== undefined ? props.today : new Date();
  const status = today < props.date ? Status.UPCOMING : Status.LAPSED;
  const prefix = getPrefix(status, props.isApproved);
  return (
    <div className={css(STYLES.container)}>
      <div aria-hidden className={css(STYLES.calendar)}/>
      <span className={css(STYLES.text)}>{prefix}</span>
      <time className={css(STYLES.text)} dateTime={formatISODate(props.date)}>
        {formatLabel(props.date)}
      </time>
      {status === Status.UPCOMING &&
        <span className={css(STYLES.text)}>
          (in {formatTimeToDate(props.date, today)})
        </span>}
    </div>);
}

enum Status {
  UPCOMING,
  LAPSED
}

function getPrefix(status: Status, isApproved: boolean): string {
  if(status === Status.UPCOMING && isApproved) {
    return 'Scheduled';
  } else if(status === Status.LAPSED && isApproved) {
    return 'Applied';
  }
  return 'Requested';
}

function formatTimeToDate(date: Date, today: Date): string {
  const diffMs = date.getTime() - today.getTime();
  const diffDays = Math.ceil(diffMs / (1000 * 60 * 60 * 24));
  if(diffDays >= 14) {
    return `${Math.floor(diffDays / 7)}w`;
  }
  return `${diffDays}d`;
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
