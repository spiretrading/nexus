import * as React from 'react';
import * as Nexus from 'nexus';
import { Transition, CSSTransition } from 'react-transition-group';
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
  isOpen: boolean;
}

export class EntitlementRow extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isOpen: false
    };
    this.showApplicabilityTable = this.showApplicabilityTable.bind(this);
  }

  public render(): JSX.Element {
    const elementSize = (() => {
      switch (this.props.breakpoint) {
        case EntitlementsPageSizing.BreakPoint.SMALL:
          return {
            ...EntitlementRow.STYLE.smallContainer
          };
        case EntitlementsPageSizing.BreakPoint.MEDIUM:
          return {
            ...EntitlementRow.STYLE.mediumContainer
          };
        case EntitlementsPageSizing.BreakPoint.LARGE:
          return {
            ...EntitlementRow.STYLE.largeContainer
          };
      }
    })();
    const buttonSize = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const nameColor = (() => {
      if (this.state.isOpen) {
        return EntitlementRow.STYLE.activeName;
      } else {
        return null;
      }
    })();
    const tableContainer = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return null;
      } else {
        return EntitlementRow.STYLE.tablePadding;
      }
    })();
    const dropDownContainer = (() => {
      if (this.state.isOpen) {
        switch (this.props.breakpoint) {
          case EntitlementsPageSizing.BreakPoint.SMALL:
            return EntitlementRow.STYLE.smallContainer;
          case EntitlementsPageSizing.BreakPoint.MEDIUM:
            return EntitlementRow.STYLE.mediumContainer;
          case EntitlementsPageSizing.BreakPoint.LARGE:
            EntitlementRow.STYLE.largeContainer;
        }
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    const padding = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.mobilePaddingStyle;
      } else {
        return EntitlementRow.STYLE.desktopPaddingStyle;
      }
    })();
    const amount = `${this.props.currencyEntry.sign}${
      this.props.entitlementEntry.price.toString()} ${
      this.props.currencyEntry.code}`;
    const amountColor = (() => {
      if (this.props.isActive) {
        return EntitlementRow.STYLE.greenCheckMark;
      } else {
        return EntitlementRow.STYLE.greyCheckMark;
      }
    })();
    const buttonRowAmountStyle = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const tableHeaderAmmount = (() => {
      if (this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();

    return (
      <VBoxLayout style={elementSize}>


        <div id='EntititlemtButtonRow' style={EntitlementRow.STYLE.header}>
          <CheckMarkButton
            size={buttonSize}
            isChecked={this.props.isActive} />
          <div style={padding} />
          <DropDownButton size={buttonSize}
            onClick={this.showApplicabilityTable} />
          <div style={padding} />
          <div style={{ ...EntitlementRow.STYLE.textBase, ...nameColor }}>
            {this.props.entitlementEntry.name}
          </div>
          <div style={EntitlementRow.STYLE.filler} />
          <div style={{
            ...EntitlementRow.STYLE.textBase,
            ...amountColor, ...buttonRowAmountStyle
          }}>
            {amount}
          </div>
        </div>
        <VBoxLayout id='hiddenDropDpwn' style={dropDownContainer}>
          <HLine color='#E6E6E6' />

          <CSSTransition in={this.state.isOpen}
            timeout={EntitlementRow.TRANSITION_LENGTH_MS}
            classNames={{
              enter: css(EntitlementRow.CSS_TRANSITION_STYLE.start),
              enterActive: css(EntitlementRow.CSS_TRANSITION_STYLE.entering),
              exit: css(EntitlementRow.CSS_TRANSITION_STYLE.exit),
              exitActive: css(EntitlementRow.CSS_TRANSITION_STYLE.exiting)
            }}>
            {(state) => (
              <div style={{...tableContainer, 
              ...EntitlementRow.STYLE.hiddenTable}}>
                <div id='Table Header' style={{
                  ...EntitlementRow.STYLE.textBase,
                  ...EntitlementRow.STYLE.header
                }}>
                  <div style={{ ...EntitlementRow.STYLE.activeName }}>
                    Applicability
              </div>
                  <div style={EntitlementRow.STYLE.filler} />
                  <div style={{ ...amountColor, ...tableHeaderAmmount }}>
                    {amount}
                  </div>
                </div>
                <div>
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
    this.setState({ isOpen: !this.state.isOpen });
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
    smallContainer: {
      minWidth: '320px',
      maxWidth: '460px'
    },
    mediumContainer: {
      minWidth: '730px',
      width: '732px'
    },
    largeContainer: {
      minWidth: '998px',
      width: '1000px'
    },
    desktopPaddingStyle: {
      width: '20px'
    },
    mobilePaddingStyle: {
      width: '18px'
    },
    tablePadding: {
      paddingLeft: '76px'
    },
    hidden: {
      visibility: 'hidden' as 'hidden'
    },
    filler: {
      flexGrow: 1
    },
    textBase: {
      font: '400 14px Roboto'
    },
    activeName: {
      color: '#4B23A0'
    },
    greenCheckMark: {
      color: '#36BB55'
    },
    greyCheckMark: {
      color: '#000000'
    },
    hiddenTable: {
      zIndex: -100
    }
  };

  private static CSS_TRANSITION_STYLE = StyleSheet.create({
    base: {
      backgroundColor: '#404040',
      width: '400px',
      padding: '10px 5px',
      marginTop: '10px',
      borderRadius: '8px'
    },
    start: {
      opacity: 0,
      transform: 'translate(0,-40px)'
    },
    entering: {
      opacity: 1,
      transition: 'opacity 1s ease',
      transform: 'translate(0,0)',
      transitionProperty: 'transform, opacity',
      transitionDuration: '1000ms'
    },
    exit: {
      opacity: 1,
      transform: 'translate(0, 0)',
      transitionProperty: 'transform, opacity',
      transitionDuration: '1000ms'
    },
    exiting: {
      opacity: 0,
      transform: 'translate(0,-40px)'
    }
  });

  private static TRANSITION_LENGTH_MS = 1000;
}
