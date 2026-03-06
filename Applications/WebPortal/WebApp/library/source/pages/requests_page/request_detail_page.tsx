import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button } from '../../components/button';
import { DateField } from '../../components/date_field';
import { PageLayout } from '../../components/page_layout';
import { RelativeDate } from '../../components/relative_date';
import { DisplaySize } from '../../display_size';
import { AccountLink } from './account_link';
import { ChangeTable } from './change_table';
import { RequestActivityItem } from './request_activity_item';
import { RequestCategoryTag } from './request_category_tag';
import { RequestEffectiveDate } from './request_effective_date';
import { RequestStateIndicator } from './request_state_indicator';

type Status = Nexus.AccountModificationRequest.Status;
const Status = Nexus.AccountModificationRequest.Status;
type Role = Nexus.AccountRoles.Role;
const Role = Nexus.AccountRoles.Role;

interface Properties {

  /** The request ID. */
  id: number;

  /** The category of the request. */
  category: Nexus.AccountModificationRequest.Type;

  /** The state of the request. */
  state: Status;

  /** When the request was created. */
  createdTime: Date;

  /** When the request was last updated. */
  updateTime: Date;

  /** The account being modified. */
  account: RequestDetailPage.AccountProfile;

  /** The account that submitted the request. */
  requester: RequestDetailPage.AccountProfile;

  /** The effective date for the change. */
  effectiveDate: Beam.Date;

  /** The list of changes. */
  changes: ChangeTable.Change[];

  /** The activity history. */
  activityList: RequestDetailPage.ActivityEntry[];

  /** The current user's access role. */
  accessRole: Role;

  /** Called when the user approves the request. */
  onApprove?: (effectiveDate: Beam.Date, comment: string) => void;

  /** Called when the user rejects the request. */
  onReject?: (comment: string) => void;
}

interface State {
  comment: string;
  effectiveDate: Beam.Date;
  containerWidth: number;
}

/** Displays the detail view for a single account modification request. */
export class RequestDetailPage extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      comment: '',
      effectiveDate: props.effectiveDate,
      containerWidth: 0
    };
    this.mainRef = React.createRef<HTMLElement>();
  }

  public render(): JSX.Element {
    return (
      <PageLayout>
        <main ref={this.mainRef} className={css(STYLES.main)}>
          {this.renderHeader()}
          <div className={css(STYLES.spacer30)}/>
          {this.renderChangesSection()}
          {this.renderActivitySection()}
          {this.renderDecisionSection()}
        </main>
      </PageLayout>);
  }

  private renderHeader(): JSX.Element {
    return (
      <header>
        <div className={css(STYLES.titleBlock)}>
          <RequestStateIndicator state={this.props.state}/>
          <h1 className={css(STYLES.h1)}>#{this.props.id}</h1>
          <RequestCategoryTag category={this.props.category}/>
          <div className={css(STYLES.updateTime)}>
            {this.state.containerWidth >= 768 &&
              <span className={css(STYLES.updatePrefix)}>Updated</span>}
            <RelativeDate datetime={this.props.updateTime}/>
          </div>
        </div>
        <div className={css(STYLES.accountRow)}>
          <AccountLink account={this.props.account.account}/>
        </div>
        <dl className={css(STYLES.dl)}>
          <dt className={css(STYLES.dt)}>Requester</dt>
          <dd className={css(STYLES.dd)}>
            <AccountLink account={this.props.requester.account}
              variant={AccountLink.Variant.AVATAR}
              initials={this.props.requester.initials}
              tint={this.props.requester.tint}/>
          </dd>
          <dt className={css(STYLES.dt)}>Created</dt>
          <dd className={css(STYLES.dd)}>
            <time dateTime={this.props.createdTime.toISOString()}>
              {formatDateTime(this.props.createdTime)}
            </time>
          </dd>
        </dl>
      </header>);
  }

  private renderChangesSection(): JSX.Element {
    const isApproved = this.props.state === Status.GRANTED;
    const showDateInput = this.canEditEffectiveDate();
    return (
      <section className={css(STYLES.section, STYLES.sectionFirst)}>
        <h2 className={css(STYLES.h2)}>Changes</h2>
        <div className={css(STYLES.spacer30)}/>
        <ChangeTable changes={this.props.changes}/>
        <div className={css(STYLES.spacer30)}/>
        <div>
          {showDateInput &&
            <div className={css(STYLES.dateField)}>
              <label htmlFor='effective-date'
                  className={css(STYLES.dateLabel)}>
                Effective Date
              </label>
              <div className={css(STYLES.dateFieldInput)}>
                <DateField displaySize={DisplaySize.SMALL}
                  value={this.state.effectiveDate}
                  onChange={this.onEffectiveDateChange}/>
              </div>
            </div>}
          <RequestEffectiveDate
            date={this.state.effectiveDate.toDate()}
            isApproved={isApproved}/>
        </div>
      </section>);
  }

  private renderActivitySection(): JSX.Element | null {
    if(this.props.accessRole === Role.TRADER) {
      return null;
    }
    return (
      <section className={css(STYLES.section)}>
        <h2 className={css(STYLES.h2)}>Activity</h2>
        <div className={css(STYLES.spacer30)}/>
        <ul className={css(STYLES.activityList)}>
          {this.props.activityList.map((entry, index) =>
            <li key={index}>
              <RequestActivityItem
                account={entry.account.account}
                activity={entry.activity}
                timestamp={entry.timestamp}
                initials={typeof entry.activity === 'string' ?
                  entry.account.initials : undefined}
                tint={typeof entry.activity === 'string' ?
                  entry.account.tint : undefined}/>
            </li>)}
        </ul>
      </section>);
  }

  private renderDecisionSection(): JSX.Element | null {
    if(this.props.state === Status.GRANTED ||
        this.props.state === Status.REJECTED) {
      return null;
    }
    const showApprove = this.canApprove();
    return (
      <section className={css(STYLES.section, STYLES.sectionLast)}>
        <textarea className={css(STYLES.textarea)}
          placeholder='Add a comment'
          value={this.state.comment}
          onChange={this.onCommentChange}/>
        <div className={css(STYLES.spacer30)}/>
        <div className={css(STYLES.actions,
            this.state.containerWidth >= 768 && STYLES.actionsWide)}>
          {showApprove &&
            <Button label='Approve' onClick={this.onApprove}/>}
          <Button label='Reject' onClick={this.onReject}/>
        </div>
      </section>);
  }

  private canEditEffectiveDate(): boolean {
    if(this.props.accessRole === Role.TRADER) {
      return false;
    }
    if(this.props.state === Status.GRANTED ||
        this.props.state === Status.REJECTED) {
      return false;
    }
    if(this.props.accessRole === Role.MANAGER &&
        this.props.state === Status.REVIEWED) {
      return false;
    }
    return true;
  }

  private canApprove(): boolean {
    if(this.props.accessRole === Role.TRADER) {
      return false;
    }
    if(this.props.accessRole === Role.MANAGER &&
        this.props.state === Status.REVIEWED) {
      return false;
    }
    return true;
  }

  private onEffectiveDateChange = (value: Beam.Date) => {
    this.setState({effectiveDate: value});
  };

  private onCommentChange = (
      event: React.ChangeEvent<HTMLTextAreaElement>) => {
    this.setState({comment: event.target.value});
  };

  private onApprove = () => {
    this.props.onApprove?.(this.state.effectiveDate, this.state.comment);
  };

  private onReject = () => {
    this.props.onReject?.(this.state.comment);
  };

  public componentDidMount() {
    this.resizeObserver = new ResizeObserver(entries => {
      for(const entry of entries) {
        this.setState({containerWidth: entry.contentRect.width});
      }
    });
    if(this.mainRef.current) {
      this.resizeObserver.observe(this.mainRef.current);
    }
  }

  public componentWillUnmount() {
    this.resizeObserver?.disconnect();
  }

  private mainRef: React.RefObject<HTMLElement>;
  private resizeObserver: ResizeObserver;
}

export namespace RequestDetailPage {

  /** Profile information for an account. */
  export interface AccountProfile {

    /** The account directory entry. */
    account: Beam.DirectoryEntry;

    /** Initials for the avatar fallback. */
    initials: string;

    /** A color for the avatar background. */
    tint: string;
  }

  /** An entry in the activity history. */
  export interface ActivityEntry {

    /** The account that performed the activity. */
    account: AccountProfile;

    /** The activity. A Status value represents a status change, and a
     *  string represents a comment. */
    activity: Nexus.AccountModificationRequest.Status | string;

    /** The timestamp of the activity. */
    timestamp: Date;
  }
}

function formatDateTime(date: Date): string {
  return date.toLocaleDateString('en-US', {
    month: 'short', day: 'numeric', year: 'numeric'
  }) + ' ' + date.toLocaleTimeString('en-US', {
    hour: '2-digit', minute: '2-digit'
  });
}

const STYLES = StyleSheet.create({
  main: {
    padding: '18px 18px 40px',
    backgroundColor: '#FFFFFF',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontWeight: 400,
    color: '#333333'
  },
  titleBlock: {
    display: 'flex',
    alignItems: 'center',
    gap: '6px'
  },
  h1: {
    fontSize: '0.875rem',
    fontWeight: 500,
    margin: 0,
    padding: 0
  },
  updateTime: {
    marginLeft: 'auto',
    display: 'flex',
    alignItems: 'center',
    gap: '4px'
  },
  updatePrefix: {
    fontSize: '0.875rem',
    color: '#7D7E90',
    textAlign: 'end'
  },
  accountRow: {
    paddingTop: '10px'
  },
  dl: {
    display: 'grid',
    gridTemplateColumns: 'auto 1fr',
    gap: '0 18px',
    margin: 0,
    paddingTop: '30px',
    alignItems: 'center'
  },
  dt: {
    fontSize: '0.875rem',
    padding: '8px 0'
  },
  dd: {
    fontSize: '0.875rem',
    margin: 0
  },
  section: {
    padding: '30px 0',
    borderBottom: '1px solid #E6E6E6'
  },
  sectionFirst: {
    paddingTop: 0
  },
  sectionLast: {
    paddingBottom: 0,
    borderBottom: 'none'
  },
  h2: {
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#4B23A0',
    margin: 0,
    padding: 0
  },
  spacer30: {
    height: '30px'
  },
  dateField: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    paddingBottom: '10px'
  },
  dateFieldInput: {
    maxWidth: '228px'
  },
  dateLabel: {
    fontSize: '0.875rem',
    color: '#333333',
    whiteSpace: 'nowrap' as 'nowrap'
  },
  activityList: {
    display: 'flex',
    flexDirection: 'column' as 'column',
    gap: '20px',
    listStyle: 'none',
    padding: 0,
    margin: 0,
    containerType: 'inline-size' as 'inline-size'
  },
  textarea: {
    width: '100%',
    minHeight: '150px',
    padding: '10px',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontSize: '0.875rem',
    color: '#333333',
    resize: 'vertical' as 'vertical',
    boxSizing: 'border-box' as 'border-box',
    '::placeholder': {
      color: '#8C8C8C'
    }
  },
  actions: {
    display: 'flex',
    flexDirection: 'column' as 'column',
    gap: '10px'
  },
  actionsWide: {
    flexDirection: 'row' as 'row',
    justifyContent: 'flex-end' as 'flex-end'
  }
});
