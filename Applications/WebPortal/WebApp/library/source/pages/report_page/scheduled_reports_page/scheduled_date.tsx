import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { highlightText } from './highlight_text';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLSpanElement>, 'children'> {

  /** The scheduled calendar date and its display label. */
  date: ScheduledDate.Date;

  /** Whether the report runs repeatedly. Defaults to false. */
  repeats?: boolean;

  /** The reference date. Defaults to the current local calendar date. */
  today?: Date;

  /** Literal text to highlight, ignoring case. */
  highlight?: string;
}

interface State {
  today: Date;
}

/** Displays a report's scheduled date and its remaining calendar days. */
export class ScheduledDate extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {today: new Date()};
    this.timer = null;
  }

  public render(): JSX.Element {
    const {date, repeats, today, highlight, className, ...attributes} =
      this.props;
    const reference = today ?? this.state.today;
    const midnight = new Date(0);
    midnight.setUTCFullYear(reference.getFullYear(), reference.getMonth(),
      reference.getDate());
    const days = (Date.parse(`${date.value}T00:00:00Z`) -
      midnight.getTime()) / 86400000;
    const prefix = (() => {
      if(days <= 0) {
        return 'Last Run';
      } else if(repeats) {
        return 'Next Run';
      }
      return 'Scheduled';
    })();
    const remaining = (() => {
      if(days >= 14) {
        return `${Math.floor(days / 7)}w`;
      }
      return `${days}d`;
    })();
    return <span {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <img src='resources/report_page/calendar.svg' alt='' width='12'
        height='12' className={css(STYLES.calendar)}/>
      <span>{highlightText(prefix, highlight)}</span>{' '}
      <time dateTime={date.value}>{highlightText(date.label, highlight)}</time>
      {days > 0 && <>
        {' '}<span>{highlightText(`(in ${remaining})`, highlight)}</span>
      </>}
    </span>;
  }

  public componentDidMount(): void {
    this.scheduleMidnight();
    window.addEventListener('focus', this.onTimeChange);
    document.addEventListener('visibilitychange', this.onTimeChange);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.today !== this.props.today) {
      this.onTimeChange();
    }
  }

  public componentWillUnmount(): void {
    window.clearTimeout(this.timer);
    window.removeEventListener('focus', this.onTimeChange);
    document.removeEventListener('visibilitychange', this.onTimeChange);
  }

  private scheduleMidnight(): void {
    window.clearTimeout(this.timer);
    this.timer = null;
    if(this.props.today) {
      return;
    }
    const now = new Date();
    const next = new Date(now.getFullYear(), now.getMonth(),
      now.getDate() + 1);
    this.timer = window.setTimeout(this.onTimeChange,
      next.getTime() - now.getTime());
  }

  private onTimeChange = () => {
    if(!this.props.today) {
      this.setState({today: new Date()});
    }
    this.scheduleMidnight();
  };

  private timer: number;
}

export namespace ScheduledDate {

  /** A scheduled calendar date and its localized label. */
  export interface Date {

    /** A valid ISO calendar date in YYYY-MM-DD format. */
    value: string;

    /** The localized text displayed for the date. */
    label: string;
  }
}

const STYLES = StyleSheet.create({
  container: {
    display: 'inline-flex',
    alignItems: 'center',
    gap: '4px',
    color: '#5D5E6D',
    fontSize: '0.875rem'
  },
  calendar: {flex: '0 0 12px'}
});
