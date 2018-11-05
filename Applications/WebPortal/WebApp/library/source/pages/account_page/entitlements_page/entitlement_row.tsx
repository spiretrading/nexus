import * as React from 'react';
import * as Nexus from 'nexus';
import {CheckMarkButton} from '.';
import {DropDownButton} from '../../../components';
import { throws } from 'assert';


interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  currencyEntry: Nexus.CurrencyDatabase.Entry; //is this needed if ED Entry has a currency????
  isSecurityActive: boolean;
}
export class EntitlementRow extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const buttonSize = '16px';
    const padding = EntitlementRow.STYLE.desktopPaddingStyle;
    const ammount = this.props.currencyEntry.sign + 
      this.props.entitlementEntry.name + ' ' + this.props.currencyEntry.code ;
    const name = 'Beep'
    const isCheckMarkChecked = (() => {
      if(this.props.isSecurityActive){
        return true;
      } else {
        return false;
      }
    })();
    const ammountColor = (() => {
      if(this.props.isSecurityActive){
        return EntitlementRow.STYLE.greenCheckMark;
      } else {
        return EntitlementRow.STYLE.greyCheckMark;
      }
    })();
    return (
        <div style={EntitlementRow.STYLE.container}>
          <CheckMarkButton 
            size={buttonSize}
            isChecked={isCheckMarkChecked}
          />
          <div style={padding}/>
          <DropDownButton size={buttonSize}/>
          <div style={padding}/>
          <div style={{...EntitlementRow.STYLE.textBase}}>
            {name}
          </div>
          <div style={EntitlementRow.STYLE.filler}/>
          <div style={{...EntitlementRow.STYLE.textBase,
              ...ammountColor}}>
            {ammount}
          </div>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      width: '100%',
      display: 'flex ' as 'flex ',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
    },
    desktopPaddingStyle: {
      width: '20px'
    },
    mobileButtonStyle:{
      width: '18px'
    },
    filler:{
       flexGrow: 1
    },
    textBase: {
      font: '400 14px Roboto',
    },
    greenCheckMark: {
      color: '#36BB55'
    },
    greyCheckMark: {
      color: '#000000'
    }
  };
}
