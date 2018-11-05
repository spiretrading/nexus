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
    const padding = EntitlementRow.STYLE.desktopPaddingStyle;
    /** 
    const buttonSize = (() => {
     if(size is mobile) {
     return '20px'
     } else {
       return '16px'
     }
     })();
    */
    const ammount = '$' +
      '100'+ ' ' +
      'USD';
    const name = 'Beep'
    const belowFoldAmmountStyle = 0; //if desktop, hidden
    const aboveFoldAmmountStyle = 0; //if mobile, hidden
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
            isChecked
            />

          <div style={padding}/>

          <DropDownButton size={buttonSize}/>

          <div style={padding}/>

          <div style={{...EntitlementRow.STYLE.textBase}}>
            {name}
          </div>

          <div style={EntitlementRow.STYLE.filler}/>

          <div style={{...EntitlementRow.STYLE.textBase}}>
            {ammount}
          </div>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex ' as 'flex ',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      //alignItems: 'flex-end' as 'flex-end',
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
    name: {
      font: '400 14px Roboto',
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
