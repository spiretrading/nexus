import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DiffBadge } from './diff_badge';

interface Properties {

  /** The name of the entitlement. */
  name: string;

  /** The action taken on the entitlement. */
  action: EntitlementsChangeItem.Action;

  /** The fee associated with the entitlement. */
  fee: Nexus.Money;

  /** The currency used to display the fee. */
  currency: Nexus.CurrencyDatabase.Entry;

  /** The fee direction for the change. If not specified, inferred from the
   *  action and fee. */
  direction?: DiffBadge.Direction;
}

interface State {
  isWide: boolean;
}

/** Displays an entitlements change item. */
export class EntitlementsChangeItem extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.containerRef = React.createRef<HTMLDivElement>();
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
    const isRevoke = this.props.action === EntitlementsChangeItem.Action.REVOKE;
    const direction = getDirection(this.props);
    return (
      <div ref={this.containerRef} className={css(STYLES.container)}>
        <div className={css(this.state.isWide ? STYLES.divWide : STYLES.div)}>
          <span className={css(STYLES.action, isRevoke ? STYLES.actionRevoke :
              STYLES.actionGrant)}>
            {isRevoke ? '-' : '+'}
          </span>
          <h2 className={css(STYLES.name, isRevoke ? STYLES.nameRevoke :
              STYLES.nameGrant, this.state.isWide && STYLES.nameWide)}>
            {this.props.name}
          </h2>
          {this.state.isWide &&
            <div className={css(STYLES.diffBadge)}>
              <DiffBadge
                value={formatBadgeValue(this.props, direction)}
                direction={direction}/>
            </div>}
        </div>
      </div>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 732;
      if(isWide !== this.state.isWide) {
        this.setState({isWide});
      }
    }
  };

  private containerRef: React.RefObject<HTMLDivElement>;
  private resizeObserver?: ResizeObserver;
}

export namespace EntitlementsChangeItem {

  /** Enumerates the action taken on an entitlement. */
  export enum Action {

    /** The entitlement is being granted. */
    GRANT,

    /** The entitlement is being revoked. */
    REVOKE
  }
}

function getDirection(props: Properties): DiffBadge.Direction {
  if(props.direction !== undefined) {
    return props.direction;
  }
  if(props.fee.equals(Nexus.Money.ZERO)) {
    return DiffBadge.Direction.NONE;
  }
  if(props.action === EntitlementsChangeItem.Action.GRANT) {
    return DiffBadge.Direction.POSITIVE;
  }
  return DiffBadge.Direction.NEGATIVE;
}

function formatBadgeValue(props: Properties,
    direction: DiffBadge.Direction): string {
  if(direction === DiffBadge.Direction.NONE &&
      props.action === EntitlementsChangeItem.Action.GRANT) {
    return 'FREE';
  }
  return props.currency.sign + props.fee.toString() + ' ' + props.currency.code;
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: 'Roboto'
  },
  div: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start'
  },
  divWide: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start'
  },
  action: {
    fontSize: '0.875rem',
    color: '#333333',
    textAlign: 'center' as 'center',
    width: '24px',
    padding: '4px 0',
    flexShrink: 0
  },
  actionGrant: {
    backgroundColor: '#A1F2C1'
  },
  actionRevoke: {
    backgroundColor: '#FFAAAB'
  },
  name: {
    fontSize: '0.875rem',
    fontWeight: 400,
    color: '#333333',
    margin: 0,
    padding: '4px 8px',
    flex: '1 1 auto',
    minWidth: 0
  },
  nameGrant: {
    backgroundColor: '#EBFFF2'
  },
  nameRevoke: {
    backgroundColor: '#FFF1F1'
  },
  nameWide: {
    flex: '0 1 auto',
    minWidth: '260px',
    maxWidth: 'calc(60ch + 16px)'
  },
  diffBadge: {
    marginLeft: '18px',
    flexShrink: 0
  }
});
