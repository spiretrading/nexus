import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, DropDownButton, HLine } from '../../..';
import { ApplicabilityTable } from './applicability_table';
import { CheckMarkButton } from './check_mark_button';

interface Properties {

  /** The entitlement to be displayed. */
  entitlementEntry: Nexus.EntitlementDatabase.Entry;

  /** The currency in which the entitlement is priced in. */
  currencyEntry: Nexus.CurrencyDatabase.Entry;

  /** Determines if the entitlement is active. */
  isActive: boolean;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** Indicates an entitlement has been clicked. */
  onClick?: () => void;
}

interface State {
  isExpanded: boolean;
  applicabilityStyle: any;
}

const TRANSITION_LENGTH_MS = 600;
const MOBILE_BUTTON_SIZE_PX = '20px';
const DESKTOP_BUTTON_SIZE_PX = '16px';

/** Displays an entitlement row with a name, amount, and applicability table. */
export class EntitlementRow extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isExpanded: false,
      applicabilityStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    };
  }

  public render(): JSX.Element {
    const buttonSize = this.props.displaySize === DisplaySize.SMALL ?
      MOBILE_BUTTON_SIZE_PX : DESKTOP_BUTTON_SIZE_PX;
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
      return this.state.isExpanded ? STYLE.amountExpanded : STYLE.name;
    })();
    const applicabilityTablePadding =
      this.props.displaySize === DisplaySize.SMALL ?
        STYLE.mobileTablePadding : STYLE.tablePadding;
    const applicabilityHeaderStyle =
      this.props.displaySize === DisplaySize.SMALL ?
        STYLE.tableHeaderSmall : STYLE.tableHeader;
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.header}>
          <CheckMarkButton size={buttonSize}
            onClick={this.props.onClick}
            isChecked={this.props.isActive}/>
          <div style={STYLE.headerPadding}/>
          <DropDownButton size={buttonSize}
            isExpanded={this.state.isExpanded}
            onClick={this.onToggle}/>
          <div style={STYLE.headerPadding}/>
          <div style={entitlementNameStyle}>
            {this.props.entitlementEntry.name}
          </div>
          <div style={STYLE.headerFiller}/>
          <div className={css(STYLES.desktopOnly)} style={amountStyle}>
            {amount}
          </div>
        </div>
        <Transition in={this.state.isExpanded}
            timeout={TRANSITION_LENGTH_MS}>
          {(state) => (
            <div ref={(el) => this.dropDownTable = el}
                className={css((this.state.applicabilityStyle as any)[state])}
                style={STYLE.expandableTable}>
              <HLine color='#E6E6E6'/>
              <div style={STYLE.header}>
                <div style={applicabilityHeaderStyle}>
                  Applicability
                </div>
                <div style={STYLE.headerFiller}/>
                <div className={css(STYLES.mobileOnly)} style={amountStyle}>
                  {amount}
                </div>
              </div>
              <div style={applicabilityTablePadding}>
                <ApplicabilityTable
                  entitlementEntry={this.props.entitlementEntry}
                  venueDatabase={this.props.venueDatabase}/>
              </div>
            </div>)}
        </Transition>
      </div>);
  }

  public componentDidMount(): void {
    let count = 0;
    for(const _ of this.props.entitlementEntry.applicability) {
      ++count;
    }
    const height = 101 + count * 40;
    this.applicabilityStyleDefinition.entering.maxHeight = `${height}px`;
    this.applicabilityStyleDefinition.entered.maxHeight = `${height}px`;
    this.setState({
      applicabilityStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    });
  }

  private onToggle = () => {
    this.setState({isExpanded: !this.state.isExpanded});
  }

  private applicabilityStyleDefinition = {
    entering: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${TRANSITION_LENGTH_MS}ms`
    },
    entered: {
      maxHeight: '0',
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
  };
  private dropDownTable: HTMLDivElement;
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
  expandableTable: {
    width: '100%',
    boxSizing: 'border-box'
  },
  tablePadding: {
    paddingLeft: '76px',
    paddingBottom: '20px',
    boxSizing: 'border-box'
  },
  mobileTablePadding: {
    paddingBottom: '20px'
  },
  name: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    color: '#000000'
  },
  nameExpanded: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#4B23A0'
  },
  activeAmount: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    color: '#36BB55'
  },
  activeAmountExpanded: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#36BB55'
  },
  amountExpanded: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#000000'
  },
  tableHeader: {
    paddingLeft: '76px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#4B23A0'
  },
  tableHeaderSmall: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#4B23A0'
  }
};

const STYLES = StyleSheet.create({
  desktopOnly: {
    display: 'none',
    '@media (min-width: 460px)': {
      display: 'block'
    }
  },
  mobileOnly: {
    '@media (min-width: 460px)': {
      display: 'none'
    }
  }
});
