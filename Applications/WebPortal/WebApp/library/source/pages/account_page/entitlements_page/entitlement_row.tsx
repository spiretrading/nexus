import { css, StyleSheet } from 'aphrodite/no-important';
import { VBoxLayout } from 'dali';
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

  /** Indicates an entitlement has been clicked.
   * @param entitlement - The entitlement that was clicked.
   */
  onClick?: () => void;
}

interface State {
  isExpanded: boolean;
  applicabilityStyle: any;
}

/** Displays a Entitlement Row. It displays a name, a amount and
 * applicability table.
 */
export class EntitlementRow extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isExpanded: false,
      applicabilityStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    };
    this.showApplicabilityTable = this.showApplicabilityTable.bind(this);
  }

  public render(): JSX.Element {
    const elementSize = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return EntitlementRow.STYLE.box.smallComponent;
        case DisplaySize.MEDIUM:
          return EntitlementRow.STYLE.box.mediumComponent;
        case DisplaySize.LARGE:
          return EntitlementRow.STYLE.box.largeComponent;
      }
    })();
    const buttonSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return EntitlementRow.MOBILE_BUTTON_SIZE_PX;
      } else {
        return EntitlementRow.DESKTOP_BUTTON_SIZE_PX;
      }
    })();
    const entitlementNameStyle = (() => {
      if(this.state.isExpanded) {
        return EntitlementRow.STYLE.text.nameWhenExpandedTable;
      } else {
        return EntitlementRow.STYLE.text.default;
      }
    })();
    const amount = (() => {
      if(this.props.entitlementEntry.price.equals(Nexus.Money.ZERO)) {
        return 'FREE';
      } else {
        return (`${this.props.currencyEntry.sign}${
          this.props.entitlementEntry.price.toString()} ${
          this.props.currencyEntry.code}`);
      }
    })();
    const amountColor = (() => {
      if(this.props.isActive) {
        if(this.state.isExpanded) {
          return EntitlementRow.STYLE.text.activeAmountWhenExpandedTable;
        } else {
          return EntitlementRow.STYLE.text.activeAmount;
        }
      } else {
        if(this.state.isExpanded) {
          return EntitlementRow.STYLE.text.amountWhenExpandedTable;
        } else {
          return EntitlementRow.STYLE.text.default;
        }
      }
    })();
    const buttonRowAmountVisibility = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return EntitlementRow.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const tableHeaderAmountVisibility = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    const applicabilityTablePadding = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return EntitlementRow.STYLE.box.mobileTablePadding;
      } else {
        return EntitlementRow.STYLE.box.tablePadding;
      }
    })();
    const applicabilityTableHeaderStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return EntitlementRow.STYLE.text.tableHeaderSmall;
      } else {
        return EntitlementRow.STYLE.text.tableHeader;
      }
    })();
    return (
      <VBoxLayout style={elementSize}>
        <div style={EntitlementRow.STYLE.box.header}>
          <CheckMarkButton
            size={buttonSize}
            onClick={this.props.onClick}
            isChecked={this.props.isActive}/>
          <div style={EntitlementRow.STYLE.box.internalHeaderPadding}/>
          <DropDownButton size={buttonSize}
            isExpanded={this.state.isExpanded}
            onClick={this.showApplicabilityTable}/>
          <div style={EntitlementRow.STYLE.box.internalHeaderPadding}/>
          <div style={entitlementNameStyle}>
            {this.props.entitlementEntry.name}
          </div>
          <div style={EntitlementRow.STYLE.box.headerFiller}/>
          <div style={{ ...amountColor, ...buttonRowAmountVisibility}}>
            {amount}
          </div>
        </div>
        <VBoxLayout>
          <Transition in={this.state.isExpanded}
              timeout={EntitlementRow.TRANSITION_LENGTH_MS}>
            {(state) => (
              <div ref={(divElement) => this.dropDownTable = divElement}
                  className={css((this.state.applicabilityStyle as any)[state])}
                  style={EntitlementRow.STYLE.box.expandableTable}>
                <HLine color='#E6E6E6'/>
                <div style={EntitlementRow.STYLE.box.header}>
                  <div style={applicabilityTableHeaderStyle}>
                    Applicability
                  </div>
                  <div style={EntitlementRow.STYLE.box.headerFiller}/>
                  <div style={{...amountColor,
                      ...tableHeaderAmountVisibility}}>
                    {amount}
                  </div>
                </div>
                <div style={applicabilityTablePadding}>
                  <ApplicabilityTable
                    entitlementEntry={this.props.entitlementEntry}
                    displaySize={this.props.displaySize}
                    venueDatabase={this.props.venueDatabase}/>
                </div>
              </div>)}
          </Transition>
        </VBoxLayout>
      </VBoxLayout>);
  }

  public componentDidMount(): void {
    const height = (() => {
      let count = 0;
      for(const entry of this.props.entitlementEntry.applicability) {
        ++count;
      }
      return 101 + count * 40;
    })();
    this.applicabilityStyleDefinition.entering.maxHeight = `${height}px`;
    this.applicabilityStyleDefinition.entered.maxHeight = `${height}px`;
    this.setState({
      applicabilityStyle: StyleSheet.create(this.applicabilityStyleDefinition)
    });
  }

  private showApplicabilityTable(): void {
    this.setState({
      isExpanded: !this.state.isExpanded
    });
  }

  private static readonly TRANSITION_LENGTH_MS = 600;
  private static readonly MOBILE_BUTTON_SIZE_PX = '20px';
  private static readonly DESKTOP_BUTTON_SIZE_PX = '16px';
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    box: {
      header: {
        height: '40px',
        boxSizing: 'border-box' as 'border-box',
        display: 'flex ' as 'flex ',
        flexDirection: 'row' as 'row',
        flexWrap: 'nowrap' as 'nowrap',
        alignItems: 'center' as 'center',
        backgroundColor: '#FFFFFF'
      },
      headerFiller: {
        flexGrow: 1
      },
      smallComponent: {
        width: '100%',
        maxWidth: '424px'
      },
      mediumComponent: {
        width: '732px',
        maxWidth: '732px'
      },
      largeComponent: {
        width: '1000px',
        maxWidth: '1000px'
      },
      internalHeaderPadding: {
        width: '18px'
      },
      expandableTable: {
        width: 'inherit' as 'inherit'
      },
      tablePadding: {
        paddingLeft: '76px',
        paddingBottom: '20px',
        width: 'inherit' as 'inherit'
      },
      mobileTablePadding: {
        paddingBottom: '20px',
        width: 'inherit' as 'inherit'
      }
    },
    text: {
      default: {
        font: '400 14px Roboto',
        color: '#000000'
      },
      activeAmount: {
        font: '400 14px Roboto',
        color: '#36BB55'
      },
      activeAmountWhenExpandedTable: {
        font: '500 14px Roboto',
        color: '#36BB55'
      },
      amountWhenExpandedTable: {
        font: '500 14px Roboto',
        color: '#000000'
      },
      nameWhenExpandedTable: {
        font: '500 14px Roboto',
        color: '#4B23A0'
      },
      tableHeader: {
        paddingLeft: '76px',
        font: '500 14px Roboto',
        color: '#4B23A0'
      },
      tableHeaderSmall: {
        font: '500 14px Roboto',
        color: '#4B23A0'
      }
    }
  };
  private applicabilityStyleDefinition = {
    entering: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${EntitlementRow.TRANSITION_LENGTH_MS}ms`
    },
    entered: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden'
    },
    exiting: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden',
      transitionProperty: 'max-height',
      transitionDuration: `${EntitlementRow.TRANSITION_LENGTH_MS}ms`
    },
    exited: {
      maxHeight: '0',
      overflow: 'hidden' as 'hidden'
    }
  };
  private dropDownTable: HTMLDivElement;
}
