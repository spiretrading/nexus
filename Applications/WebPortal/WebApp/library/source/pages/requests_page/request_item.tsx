import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { RelativeDate } from '../../components/relative_date';
import { EntitlementsChangeItem } from './entitlements_change_item';
import { RequestCategoryTag } from './request_category_tag';
import { RequestEffectiveDate } from './request_effective_date';
import { RequestStateIndicator } from './request_state_indicator';
import type { RequestsModel } from './requests_model';
import { RiskControlsChangeItem } from './risk_controls_change_item';

type Status = Nexus.AccountModificationRequest.Status;
const Status = Nexus.AccountModificationRequest.Status;

interface Properties {

  /** The unique identifier of the request. */
  id: number;

  /** The category of the request. */
  category: Nexus.AccountModificationRequest.Type;

  /** The state of the request. */
  state: Status;

  /** The time the request was last updated. */
  updateTime: Date;

  /** The account username on which the change is requested. */
  accountName: string;

  /** The date when the change is scheduled. */
  effectiveDate: Date;

  /** The first change requested. */
  firstChange: RequestsModel.ListChange;

  /** The number of additional changes beyond the first. */
  additionalChangesCount: number;

  /** The number of comments on the request. */
  commentCount: number;

  /** The manager approval details. Only present when state is REVIEWED. */
  managerApproval?: RequestsModel.ManagerApproval;
}

interface State {
  isWide: boolean;
}

/** Displays a summary of a change request as a clickable list item. */
export class RequestItem extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.containerRef = React.createRef<HTMLAnchorElement>();
    this.state = {
      isWide: false
    };
  }

  public componentDidMount(): void {
    this.resizeObserver = new ResizeObserver(this.onResize);
    if(this.containerRef.current) {
      this.resizeObserver.observe(this.containerRef.current);
    }
  }

  public componentWillUnmount(): void {
    this.resizeObserver?.disconnect();
  }

  public render(): JSX.Element {
    const showEffectiveDate = this.props.state !== Status.REJECTED;
    const showCommentCount = this.props.commentCount > 0;
    return (
      <a ref={this.containerRef} className={css(STYLES.link)}
          href={`requests/${this.props.id}`}>
        <div className={css(STYLES.header)}>
          <RequestStateIndicator state={this.props.state}/>
          <h2 className={css(STYLES.id)}>#{this.props.id}</h2>
          <RequestCategoryTag category={this.props.category}/>
          <div className={css(STYLES.updateTime,
              this.state.isWide && STYLES.updateTimeWide)}>
            {this.state.isWide &&
              <span className={css(STYLES.prefix)}>Updated</span>}
            <span className={css(STYLES.relativeDate,
                this.state.isWide && STYLES.relativeDateWide)}>
              <RelativeDate datetime={this.props.updateTime}/>
            </span>
          </div>
        </div>
        <div className={css(STYLES.metadata)}>
          <span className={css(STYLES.account)}>
            {this.props.accountName}
          </span>
          {this.state.isWide && showEffectiveDate &&
            <div className={css(STYLES.dateBlock)}>
              <RequestEffectiveDate date={this.props.effectiveDate}
                isApproved={this.props.state === Status.GRANTED ||
                  this.props.state === Status.SCHEDULED}/>
            </div>}
          {this.props.managerApproval &&
            <div className={css(STYLES.decisionBlock)}>
              <img aria-hidden='true' width='16' height='16'
                className={css(STYLES.decisionIcon)}
                src='resources/requests_page/checkmark.svg'/>
              <span className={css(STYLES.decisionText)}>
                {this.props.managerApproval.self ? 'You approved' :
                  `${this.props.managerApproval.approver} approved`}
              </span>
            </div>}
          {!this.state.isWide && showEffectiveDate &&
            !this.props.managerApproval &&
            <div className={css(STYLES.dateBlock)}>
              <RequestEffectiveDate date={this.props.effectiveDate}
                isApproved={this.props.state === Status.GRANTED ||
                  this.props.state === Status.SCHEDULED}/>
            </div>}
        </div>
        <div className={css(STYLES.content)}>
          <div className={css(STYLES.changes)}>
            {renderFirstChange(this.props)}
            {this.props.additionalChangesCount > 0 &&
              <span className={css(STYLES.additionalChanges)}>
                +{this.props.additionalChangesCount} more
              </span>}
          </div>
          {this.state.isWide && showCommentCount &&
            <div className={css(STYLES.commentBlock)}>
              <img aria-hidden='true' width='16' height='16'
                className={css(STYLES.commentIcon)}
                src='resources/requests_page/comment.svg'/>
              <span className={css(STYLES.commentCount)}
                  aria-label={`${this.props.commentCount} comments`}>
                {this.props.commentCount}
              </span>
            </div>}
        </div>
      </a>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 768;
      if(isWide !== this.state.isWide) {
        this.setState({isWide});
      }
    }
  };

  private containerRef: React.RefObject<HTMLAnchorElement>;
  private resizeObserver?: ResizeObserver;
}

function renderFirstChange(props: Properties): JSX.Element {
  const change = props.firstChange;
  if(change.type === 'risk_controls') {
    return (
      <RiskControlsChangeItem
        name={change.name}
        oldValue={change.oldValue}
        newValue={change.newValue}
        delta={change.delta}/>);
  }
  return (
    <EntitlementsChangeItem
      name={change.name}
      action={change.action}
      fee={change.fee}
      currency={change.currency}
      direction={change.direction}/>);
}

const STYLES = StyleSheet.create({
  link: {
    display: 'block',
    textDecoration: 'none',
    fontFamily: 'Roboto',
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '11px 17px',
    color: 'inherit',
    ':hover': {
      backgroundColor: '#F8F8F8'
    },
    ':focus-visible': {
      borderColor: '#684BC7',
      outline: 'none'
    }
  },
  header: {
    display: 'flex',
    alignItems: 'center' as 'center',
    gap: '6px'
  },
  id: {
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#333333',
    margin: 0,
    whiteSpace: 'nowrap' as 'nowrap'
  },
  updateTime: {
    display: 'flex',
    alignItems: 'center' as 'center',
    marginLeft: 'auto',
    flexShrink: 0
  },
  updateTimeWide: {
    gap: '4px'
  },
  prefix: {
    fontSize: '0.875rem',
    color: '#7D7E90',
    textAlign: 'end' as 'end'
  },
  relativeDate: {
    textTransform: 'capitalize' as 'capitalize',
    textAlign: 'end' as 'end'
  },
  relativeDateWide: {
    textTransform: 'none' as 'none'
  },
  metadata: {
    display: 'flex',
    alignItems: 'center' as 'center',
    marginTop: '4px'
  },
  account: {
    fontSize: '0.875rem',
    color: '#333333'
  },
  dateBlock: {
    marginLeft: '18px',
    flexShrink: 0
  },
  decisionBlock: {
    display: 'flex',
    alignItems: 'center' as 'center',
    marginLeft: '18px',
    flexShrink: 0
  },
  decisionIcon: {
    flexShrink: 0
  },
  decisionText: {
    fontSize: '0.875rem',
    color: '#7D7E90',
    marginLeft: '4px'
  },
  content: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start',
    marginTop: '10px'
  },
  changes: {
    flex: '1 1 auto',
    minWidth: 0
  },
  additionalChanges: {
    fontSize: '0.875rem',
    color: '#5D5E6D',
    padding: '4px 0 0'
  },
  commentBlock: {
    display: 'flex',
    alignItems: 'center' as 'center',
    marginLeft: '18px',
    flexShrink: 0
  },
  commentIcon: {
    flexShrink: 0
  },
  commentCount: {
    fontSize: '0.875rem',
    color: '#7D7E90',
    padding: '4px 0',
    marginLeft: '4px'
  }
});
