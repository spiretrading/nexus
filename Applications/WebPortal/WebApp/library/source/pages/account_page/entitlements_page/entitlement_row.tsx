import * as React from 'react';
import * as Nexus from 'nexus';
import { CSSTransition } from 'react-transition-group';
import { css, StyleSheet } from 'aphrodite/no-important';
import { CheckMarkButton } from '.';
import { DropDownButton, HLine } from '../../../components';
import { EntitlementsPageSizing } from './entitlements_page';
import { EntitlementTable } from './entitlement_table';
import { VBoxLayout } from '../../../layouts';

interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  currencyEntry: Nexus.CurrencyDatabase.Entry;
  isActive: boolean;
  breakpoint: EntitlementsPageSizing.BreakPoint;
  marketDatabase: Nexus.MarketDatabase;
}

interface State {
  isExpanded: boolean;
}

export class EntitlementRow extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isExpanded: false
    };
    this.showApplicabilityTable = this.showApplicabilityTable.bind(this);
  }

  public render(): JSX.Element {
    const elementSize = (() => {
      switch (this.props.breakpoint) {
        case EntitlementsPageSizing.BreakPoint.SMALL:
          return EntitlementRow.STYLE.box.smallComponent;
        case EntitlementsPageSizing.BreakPoint.MEDIUM:
          return EntitlementRow.STYLE.box.mediumComponent;
        case EntitlementsPageSizing.BreakPoint.LARGE:
          return EntitlementRow.STYLE.box.largeComponent;
      }
    })(); // good
    const buttonSize = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })(); // good
    const entitlementNameStyle = (() => {
      if (this.state.isExpanded) {
        return EntitlementRow.STYLE.text.nameWhenExpandedTable;
      } else {
        return EntitlementRow.STYLE.text.basic;
      }
    })();
    const headerPaddingInternal = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.box.mobilePaddingStyle;
      } else {
        return EntitlementRow.STYLE.box.paddingStyle;
      }
    })();
    const amount = `${this.props.currencyEntry.sign}${
      this.props.entitlementEntry.price.toString()} ${
      this.props.currencyEntry.code}`;
    const amountColor = (() => {
      if (this.props.isActive) {
        if (this.state.isExpanded) {
          return EntitlementRow.STYLE.text.activeAmmountWhenExpandedTable;
        } else {
          return EntitlementRow.STYLE.text.activeAmmount;
        }
      } else {
        return EntitlementRow.STYLE.text;
      }
    })();
    const buttonRowAmountVisibility = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.hidden;
      } else {
        return null;
      }
    })(); // needs better name
    const tableHeaderAmmountVisibility = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })(); // needs better name
    const applicabilityTablePadding = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.box.mobileTablePadding;
      } else {
        return EntitlementRow.STYLE.box.tablePadding;
      }
    })(); // needs better name
    const lineVisibility = (() => {
      if (this.state.isExpanded) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    return (
      <VBoxLayout id='EntititlemtButtonRow' style={elementSize}>
        <div id='EntititlemtButtonRow' style={EntitlementRow.STYLE.header}>
          <CheckMarkButton
            size={buttonSize}
            isChecked={this.props.isActive} />
          <div style={headerPaddingInternal} />
          <DropDownButton size={buttonSize}
            onClick={this.showApplicabilityTable} />
          <div style={headerPaddingInternal} />
          <div style={entitlementNameStyle}>
            {this.props.entitlementEntry.name}
          </div>
          <div style={EntitlementRow.STYLE.box.headerFiller} />
          <div style={{
            ...amountColor, ...buttonRowAmountVisibility
          }}>
            {amount}
          </div>
        </div>
        <VBoxLayout id='hiddenDropDpwn'>
          <div style={lineVisibility}>
            <HLine color='#E6E6E6' />
          </div>
          <CSSTransition in={this.state.isExpanded}
            timeout={EntitlementRow.TRANSITION_LENGTH_MS}
            classNames={{
              enter: css(EntitlementRow.SLIDE_TRANSITION_STYLE.start),
              enterActive: css(EntitlementRow.SLIDE_TRANSITION_STYLE.entering),
              exit: css(EntitlementRow.SLIDE_TRANSITION_STYLE.exit),
              exitActive: css(EntitlementRow.SLIDE_TRANSITION_STYLE.exiting),
              exitDone: css(EntitlementRow.SLIDE_TRANSITION_STYLE.end)
            }}>
            {(state) => (
              <div className={css(EntitlementRow.SLIDE_TRANSITION_STYLE.end)}
                style={{ ...EntitlementRow.STYLE.hiddenTable }}>
                <div id='Table Header' style={{
                  ...EntitlementRow.STYLE.header
                }}>
                  <div style={entitlementNameStyle}>
                    Applicability
                </div>
                  <div style={EntitlementRow.STYLE.box.headerFiller} />
                  <div style={{
                    ...amountColor,
                    ...tableHeaderAmmountVisibility
                  }}>
                    {amount}
                  </div>
                </div>
                <div style={applicabilityTablePadding}>
                  <EntitlementTable
                    entitlementEntry={this.props.entitlementEntry}
                    breakpoint={this.props.breakpoint}
                    marketDatabase={this.props.marketDatabase} />
                </div>
              </div>
            )}
          </CSSTransition>
        </VBoxLayout>
      </VBoxLayout>);
  }

  private showApplicabilityTable(): void {
    this.setState({
      isExpanded: !this.state.isExpanded
    });
  }

  private static readonly STYLE = {
    header: {
      height: '40px',
      boxSizing: 'border-box' as 'border-box',
      display: 'flex ' as 'flex ',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      backgroundColor: '#FFFFFF'
    },
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    hiddenTable: {
      zIndex: -100,
      width: 'inherit' as 'inherit'
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
        minWidth: '284px',
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
      paddingStyle: {
        width: '20px'
      },
      mobilePaddingStyle: {
        width: '18px'
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
      basic: {
        font: '400 14px Roboto',
        color: '#000000'
      },
      activeAmmount: {
        font: '400 14px Roboto',
        color: '#36BB55'
      },
      activeAmmountWhenExpandedTable: {
        font: '500 14px Roboto',
        color: '#36BB55'
      },
      nameWhenExpandedTable: {
        font: '400 14px Roboto',
        color: '#4B23A0'
      }
    }
  };

  private static SLIDE_TRANSITION_STYLE = StyleSheet.create({
    start: {
      opacity: 0,
      marginTop: '-60px',
      marginBottom: '0px',
      transform: 'translate(0,0)',
      visibility: 'visible' as 'visible'
    },
    entering: {
      opacity: 1,
      marginTop: '0px',
      marginBottom: '0px',
      transform: 'translate(0,0)',
      transitionProperty: 'transform, opacity, margin',
      transitionDuration: '200ms'
    },
    exit: {
      opacity: 0,
      marginTop: '0px',
      marginBottom: '0px'
    },
    exiting: {
      opacity: 0,
      marginTop: '-200px',
      marginBottom: '0px',
      transition: 'opacity 100ms, margin 200ms'
    },
    end: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  });

  private static TRANSITION_LENGTH_MS = 1000;
}
