import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { AccountLink } from './account_link';
import { RelativeDate } from '../../components/relative_date';

interface Properties {

  /** The account that performed the activity. */
  account: Beam.DirectoryEntry;

  /** The type of activity. */
  type: RequestActivityItem.Type;

  /** The timestamp of the activity. */
  timestamp: Date;

  /** The approval stage. Required when type is APPROVED. */
  stage?: RequestActivityItem.ApprovalStage;

  /** The comment body text. Required when type is COMMENT. */
  text?: string;

  /** Initials for the account avatar. Required when type is COMMENT. */
  initials?: string;

  /** A color for the account avatar. Required when type is COMMENT. */
  tint?: string;
}

/** Displays a single activity entry in a request's history. */
export class RequestActivityItem extends React.Component<Properties> {
  public render(): JSX.Element {
    const isComment = this.props.type === RequestActivityItem.Type.COMMENT;
    return (
      <div className={css(STYLES.article)}>
        <div className={css(STYLES.header)}>
          {this.renderAccountLink()}
          {!isComment && this.renderAction()}
          {!isComment &&
            <span className={css(STYLES.delimiter)}>{' \u2022 '}</span>}
          <RelativeDate datetime={this.props.timestamp}/>
        </div>
        {isComment && this.props.text &&
          <p className={css(STYLES.text)}>{this.props.text}</p>}
      </div>);
  }

  private renderAccountLink(): JSX.Element {
    if(this.props.type === RequestActivityItem.Type.COMMENT) {
      return (
        <AccountLink account={this.props.account}
          variant={AccountLink.Variant.AVATAR}
          initials={this.props.initials} tint={this.props.tint}/>);
    }
    return <AccountLink account={this.props.account}/>;
  }

  private renderAction(): JSX.Element {
    return (
      <span className={css(STYLES.action)}>
        {getActionText(this.props.type, this.props.stage)}
      </span>);
  }
}

export namespace RequestActivityItem {

  /** Enumerates the types of request activity. */
  export enum Type {

    /** A change request was created. */
    CREATED,

    /** A user approved the request. */
    APPROVED,

    /** A user rejected the request. */
    REJECTED,

    /** A user left a comment. */
    COMMENT
  }

  /** Enumerates the stages of an approval. */
  export enum ApprovalStage {

    /** Approval from a manager. */
    MANAGER,

    /** Approval from an admin. */
    FINAL
  }
}

function getActionText(type: RequestActivityItem.Type,
    stage?: RequestActivityItem.ApprovalStage): string {
  switch(type) {
    case RequestActivityItem.Type.CREATED:
      return 'created this request';
    case RequestActivityItem.Type.APPROVED:
      switch(stage) {
        case RequestActivityItem.ApprovalStage.MANAGER:
          return 'approved \u2014 Manager';
        case RequestActivityItem.ApprovalStage.FINAL:
          return 'approved \u2014 Final';
        default:
          return 'approved';
      }
    case RequestActivityItem.Type.REJECTED:
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
