import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { AccountLink } from './account_link';
import { RelativeDate } from '../../components/relative_date';

type Status = Nexus.AccountModificationRequest.Status;
const Status = Nexus.AccountModificationRequest.Status;

interface Properties {

  /** The account that performed the activity. */
  account: Beam.DirectoryEntry;

  /** The activity. A Status value represents a status change, and a
   *  string represents a comment. */
  activity: Status | string;

  /** The timestamp of the activity. */
  timestamp: Date;

  /** Initials for the account avatar. Required when type is a comment. */
  initials?: string;

  /** A color for the account avatar. Required when type is a comment. */
  tint?: string;
}

/** Displays a single activity entry in a request's history. */
export function RequestActivityItem(props: Properties) {
  const isComment = typeof props.activity === 'string';
  return (
    <div className={css(STYLES.article)}>
      <div className={css(STYLES.header)}>
        {renderAccountLink(props, isComment)}
        {!isComment && renderAction(props.activity as Status)}
        {!isComment &&
          <span className={css(STYLES.delimiter)}>{' \u2022 '}</span>}
        <RelativeDate datetime={props.timestamp}/>
      </div>
      {isComment && <p className={css(STYLES.text)}>{props.activity}</p>}
    </div>);
}

function renderAccountLink(props: Properties, isComment: boolean): JSX.Element {
  if(isComment) {
    return (
      <AccountLink account={props.account}
        variant={AccountLink.Variant.AVATAR}
        initials={props.initials} tint={props.tint}/>);
  }
  return <AccountLink account={props.account}/>;
}

function renderAction(status: Status): JSX.Element {
  return (
    <span className={css(STYLES.action)}>
      {getActionText(status)}
    </span>);
}

function getActionText(status: Status): string {
  switch(status) {
    case Status.PENDING:
      return 'created this request';
    case Status.REVIEWED:
      return 'approved \u2014 Manager';
    case Status.SCHEDULED:
      return 'approved \u2014 Final';
    case Status.GRANTED:
      return 'granted';
    case Status.REJECTED:
      return 'rejected';
    default:
      return '';
  }
}

const STYLES = StyleSheet.create({
  article: {
    display: 'flex',
    flexDirection: 'column' as 'column'
  },
  header: {
    display: 'flex',
    alignItems: 'center' as 'center',
    gap: '6px',
    padding: '4px 0'
  },
  action: {
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    color: '#7D7E90'
  },
  delimiter: {
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    color: '#7D7E90'
  },
  text: {
    margin: 0,
    marginTop: '6px',
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    color: '#333333'
  }
});
