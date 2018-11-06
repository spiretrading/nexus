import * as React from 'react';
import * as Nexus from 'nexus';
import {CheckMarkButton} from '.';
import {DropDownButton} from '../../../components';
import {EntitlementsPageSizing} from './entitlements_page';

interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  currencyEntry: Nexus.CurrencyDatabase.Entry;
  isSecurityActive: boolean;
  breakpoint: EntitlementsPageSizing.BreakPoint;
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
    const containerStyle = (() => {
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
    const padding = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.mobilePaddingStyle;
      } else {
        return EntitlementRow.STYLE.desktopPaddingStyle;
      }
    })();
    const ammount = this.props.currencyEntry.sign +
      this.props.entitlementEntry.price.toString() + ' ' +
      this.props.currencyEntry.code ;
    const amount = `${this.props.currencyEntry.sign}${
      this.props.entitlementEntry.price.toString()} ${
      this.props.currencyEntry.code}`;
    const amountColor = (() => {
      if(this.props.isSecurityActive) {
        return EntitlementRow.STYLE.greenCheckMark;
      } else {
        return EntitlementRow.STYLE.greyCheckMark;
      }
    })();
    const buttonRowAmountStyle = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementRow.STYLE.hiddenText;
      } else {
        return null;
      }
    })();
    return (
      <div style={containerStyle}>
        <CheckMarkButton
          size={buttonSize}
          isChecked={this.props.isSecurityActive}/>
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
    hiddenText: {
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
