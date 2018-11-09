import * as React from 'react';
import * as Nexus from 'nexus';
import {CheckMarkButton} from '.';
import {DropDownButton, HLine} from '../../../components';
import {EntitlementsPageSizing} from './entitlements_page';
import { EntitlementTable } from './entitlement_table';

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
    const topRowStyle = (() => {
      switch(this.props.breakpoint) {
        case EntitlementsPageSizing.BreakPoint.SMALL:
          return {...EntitlementRow.STYLE.container,
            ...EntitlementRow.STYLE.smallContainer};
        case EntitlementsPageSizing.BreakPoint.MEDIUM:
          return {...EntitlementRow.STYLE.container,
            ...EntitlementRow.STYLE.mediumContainer};
        case EntitlementsPageSizing.BreakPoint.LARGE:
          return {...EntitlementRow.STYLE.container,
            ...EntitlementRow.STYLE.largeContainer};
      }
    })();
    const buttonSize = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const nameColor = (() => {
      if(this.state.isOpen) {
        return EntitlementRow.STYLE.activeName;
      } else {
        return null;
      }
    })();
    const table = (() => {
      if(this.state.isOpen) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    const tableHeader = (() => {
      if(this.state.isOpen) {
        return topRowStyle;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    const padding = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.mobilePaddingStyle;
      } else {
        return EntitlementRow.STYLE.desktopPaddingStyle;
      }
    })();
    const amount = `${this.props.currencyEntry.sign}${
      this.props.entitlementEntry.price.toString()} ${
      this.props.currencyEntry.code}`;
    const amountColor = (() => {
      if(this.props.isActive) {
        return EntitlementRow.STYLE.greenCheckMark;
      } else {
        return EntitlementRow.STYLE.greyCheckMark;
      }
    })();
    const buttonRowAmountStyle = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const tableHeaderAmmount = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return null;
      } else {
        return EntitlementRow.STYLE.hidden;
      }
    })();
    return (
      <div>
        <div style={topRowStyle}>
          <CheckMarkButton
            size={buttonSize}
            isChecked={this.props.isActive}/>
          <div style={padding}/>
          <DropDownButton size={buttonSize}
            onClick={this.showApplicabilityTable}/>
          <div style={padding}/>
          <div style={{...EntitlementRow.STYLE.textBase, ...nameColor}}>
            {this.props.entitlementEntry.name}
          </div>
          <div style={EntitlementRow.STYLE.filler}/>
          <div style={{...EntitlementRow.STYLE.textBase,
              ...amountColor, ...buttonRowAmountStyle}}>
            {amount}
          </div>
        </div>
        <div style= {table}>
         <HLine color='#E6E6E6'/>
            <div style={tableHeader}>
              <div style={{...EntitlementRow.STYLE.textBase,
                ...EntitlementRow.STYLE.activeName}}>
                  Applicability
              </div>
              <div style={EntitlementRow.STYLE.filler}/>
              <div style={{...EntitlementRow.STYLE.textBase,
                  ...amountColor, ...tableHeaderAmmount}}>
                {amount}
              </div>
            </div>
          <EntitlementTable
          entitlementEntry={this.props.entitlementEntry}
          breakpoint={this.props.breakpoint}
          marketDatabase={this.props.marketDatabase}/>
        </div>
      </div>);
  }

  private showApplicabilityTable(): void {
    this.setState({ isOpen: !this.state.isOpen});
  }

  private static readonly STYLE = {
    container: {
      height: '40px',
      boxSizing: 'border-box' as 'border-box',
      display: 'flex ' as 'flex ',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    smallContainer: {
      minWidth: '320px',
      maxWidth: '460px'
    },
    mediumContainer: {
      width: '732px'
    },
    largeContainer:{
      width: '1000px'
    },
    desktopPaddingStyle: {
      width: '20px'
    },
    mobilePaddingStyle:{
      width: '18px'
    },
    hidden: {
      visibility: 'hidden' as 'hidden'
    },
    filler:{
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
    }
  };
}
