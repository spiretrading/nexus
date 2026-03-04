import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The date and time to display. */
  datetime: Date;

  /** The current date. Defaults to today. */
  today?: Date;
}

/** Displays a date/time relative to today. */
export class RelativeDate extends React.Component<Properties> {
  public render(): JSX.Element {
    const today = this.props.today !== undefined ?
      this.props.today : new Date();
    const relativeDay = this.getRelativeDay(today);
    const content = (() => {
      switch(relativeDay) {
        case RelativeDay.TODAY:
          return this.props.datetime.toLocaleTimeString('en-US', {
            hour: '2-digit', minute: '2-digit', hour12: false
          });
        case RelativeDay.YESTERDAY:
          return 'yesterday';
        case RelativeDay.OTHER_DAY:
          return this.props.datetime.toLocaleDateString('en-US', {
            month: 'short', day: 'numeric', year: 'numeric'
          });
      }
    })();
    return (
      <time className={css(STYLES.time)}
        dateTime={this.props.datetime.toISOString()}>
        {content}
      </time>);
  }

  private getRelativeDay(today: Date): RelativeDay {
    const dtYear = this.props.datetime.getFullYear();
    const dtMonth = this.props.datetime.getMonth();
    const dtDate = this.props.datetime.getDate();
    const tYear = today.getFullYear();
    const tMonth = today.getMonth();
    const tDate = today.getDate();
    if(dtYear === tYear && dtMonth === tMonth && dtDate === tDate) {
      return RelativeDay.TODAY;
    }
    const yesterday = new Date(tYear, tMonth, tDate - 1);
    if(dtYear === yesterday.getFullYear() &&
        dtMonth === yesterday.getMonth() &&
        dtDate === yesterday.getDate()) {
      return RelativeDay.YESTERDAY;
    }
    return RelativeDay.OTHER_DAY;
  }
}

enum RelativeDay {
  TODAY,
  YESTERDAY,
  OTHER_DAY
}

const STYLES = StyleSheet.create({
  time: {
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    color: '#7D7E90'
  }
});
