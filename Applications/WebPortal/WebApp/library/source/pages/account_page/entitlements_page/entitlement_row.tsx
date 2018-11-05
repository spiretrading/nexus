import * as React from 'react';
import * as Nexus from 'nexus';
import {CheckMarkButton} from '.';
import {DropDownButton} from '../../../components';


interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  currencyEntry?: Nexus.CurrencyDatabase.Entry; //is this needed if ED Entry has a currency????
  isSecurityActive: boolean;
}
export class EntitlementRow extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const buttonSize = '16px'; // can be 16 or 20 px
    /** 
    const buttonSize = (() => {
     if(size is mobile) {
     return '20px'
     } else {
       return '16px'
     }
     })();
    */
    const ammount = this.props.currencyEntry.sign +
      this.props.entitlementEntry.price + ' ' +
      this.props.currencyEntry.code;
    const belowFoldAmmountStyle = 0; //if desktop, hidden
    const aboveFoldAmmountStyle = 0; //if mobile, hidden
    const ammountColor = (() => {
      if(this.props.entitlementEntry){
        return EntitlementRow.STYLE.greenCheckMark;
      } else {
        return EntitlementRow.STYLE.greyCheckMark;
      }
    })();
    return (
      <div>
        <div style={EntitlementRow.STYLE.container}>
          <CheckMarkButton 
            size={buttonSize}
            isReadOnly //???????????
            //isChecked? 
            />
          <DropDownButton size='20px'/>
          <div style={EntitlementRow.STYLE.textBase}>
            {name}
          </div>
          <div style={{...EntitlementRow.STYLE.textBase}}>
            {ammount}
          </div>
        </div>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'flex-end' as 'flex-end',
    },
    desktopButtonStyle: {
      paddingLeft: '20px'
    },
    mobileButtonStyle:{
      paddingLeft: '18px'
    },

    textBase: {
      flexBasis: '0',
      font: 'Roboto',
      fontSize: '14px'
    },
    greenCheckMark: {
      color: '#36BB55'
    },
    greyCheckMark: {
      color: '#000000'
    }
  };
}
