import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { Checkbox, DropDownButton } from '../../..';
import { ApplicabilityTable } from './applicability_table';

interface Properties {

  /** The entitlement to be displayed. */
  entitlementEntry: Nexus.EntitlementDatabase.Entry;

  /** The currency in which the entitlement is priced in. */
  currencyEntry: Nexus.CurrencyDatabase.Entry;

  /** Determines if the entitlement is active. */
  isActive: boolean;

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** Indicates an entitlement has been clicked. */
  onClick?: () => void;
}

interface State {
  isExpanded: boolean;
  transitionStyles: {
    entering: object;
    entered: object;
    exiting: object;
    exited: object;
  };
}

const TRANSITION_LENGTH_MS = 600;
const BUTTON_SIZE_PX = '16px';

function computeTransitionStyles(
    entitlementEntry: Nexus.EntitlementDatabase.Entry) {
  let count = 0;
  for(const _ of entitlementEntry.applicability) {
    ++count;
  }
  const maxHeight = `${101 + count * 40}px`;
  return StyleSheet.create({
    entering: {
      maxHeight,
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${TRANSITION_LENGTH_MS}ms`
    },
    entered: {
      maxHeight,
      overflow: 'hidden' as 'hidden'
    },
    exiting: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${TRANSITION_LENGTH_MS}ms`
    },
    exited: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden'
    }
  });
}

/** Displays an entitlement row with a name, amount, and applicability table. */
export class EntitlementRow extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isExpanded: false,
      transitionStyles: computeTransitionStyles(props.entitlementEntry)
    };
  }

  public render(): JSX.Element {
    const entitlementNameStyle = this.state.isExpanded ?
      STYLE.nameExpanded : STYLE.name;
    const amount = this.props.entitlementEntry.price.equals(Nexus.Money.ZERO) ?
      'FREE' :
      `${this.props.currencyEntry.sign}${
        this.props.entitlementEntry.price.toString()} ${
        this.props.currencyEntry.code}`;
    const amountStyle = (() => {
      if(this.props.isActive) {
        return this.state.isExpanded ?
          STYLE.activeAmountExpanded : STYLE.activeAmount;
      }
      return this.state.isExpanded ? STYLE.amountExpanded : undefined;
    })();
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.header}>
          <Checkbox
            id={`${this.props.entitlementEntry.name}-active`}
            name={`${this.props.entitlementEntry.name}-active`}
            aria-label={`Toggle ${this.props.entitlementEntry.name} activation`}
            checked={this.props.isActive}
            onClick={this.props.onClick}/>
          <div style={STYLE.headerPadding}/>
          <DropDownButton size={BUTTON_SIZE_PX}
            isExpanded={this.state.isExpanded}
            onClick={this.onToggle}/>
          <div style={STYLE.headerPadding}/>
          <h2 style={entitlementNameStyle}>
            {this.props.entitlementEntry.name}
          </h2>
          <div style={STYLE.headerFiller}/>
          <span className={css(STYLES.desktopOnly)} style={amountStyle}>
            {amount}
          </span>
        </div>
        <Transition in={this.state.isExpanded}
            timeout={TRANSITION_LENGTH_MS}>
          {(state) => (
            <div className={css(
                (this.state.transitionStyles as any)[state])}>
              <div className={css(STYLES.applicabilitySection)}>
                <div style={STYLE.header}>
                  <h3 style={STYLE.tableHeader}>
                    Applicability
                  </h3>
                  <div style={STYLE.headerFiller}/>
                  <span className={css(STYLES.mobileOnly)} style={amountStyle}>
                    {amount}
                  </span>
                </div>
                <ApplicabilityTable
                  entitlementEntry={this.props.entitlementEntry}
                  venueDatabase={this.props.venueDatabase}/>
              </div>
            </div>)}
        </Transition>
      </div>);
  }

  private onToggle = () => {
    this.setState({isExpanded: !this.state.isExpanded});
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    display: 'flex',
    flexDirection: 'column',
    width: '100%'
  },
  header: {
    height: '40px',
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'row',
    flexWrap: 'nowrap',
    alignItems: 'center',
    backgroundColor: '#FFFFFF'
  },
  headerFiller: {
    flexGrow: 1
  },
  headerPadding: {
    width: '18px'
  },
  name: {
    fontSize: 'inherit',
    fontWeight: 400,
    margin: 0
  },
  nameExpanded: {
    fontSize: 'inherit',
    fontWeight: 500,
    color: '#4B23A0',
    margin: 0
  },
  activeAmount: {
    color: '#36BB55'
  },
  activeAmountExpanded: {
    fontWeight: 500,
    color: '#36BB55'
  },
  amountExpanded: {
    fontWeight: 500
  },
  tableHeader: {
    fontSize: 'inherit',
    fontWeight: 500,
    color: '#4B23A0',
    margin: 0
  }
};

const STYLES = StyleSheet.create({
  applicabilitySection: {
    borderTop: '1px solid #E6E6E6',
    paddingBottom: '20px',
    width: '100%',
    boxSizing: 'border-box',
    '@media (min-width: 768px)': {
      paddingLeft: '76px'
    }
  },
  desktopOnly: {
    display: 'none',
    '@media (min-width: 768px)': {
      display: 'block'
    }
  },
  mobileOnly: {
    '@media (min-width: 768px)': {
      display: 'none'
    }
  }
});
