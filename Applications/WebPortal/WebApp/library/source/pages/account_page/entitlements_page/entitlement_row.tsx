import * as React from 'react';
import * as Nexus from 'nexus';
import {CheckMarkButton} from '.';
import {DropDownButton} from '../../../components';


interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  currencyEntry?: Nexus.CurrencyDatabase.Entry;
  isSecurityActive: boolean;
}
export class EntitlementRow extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const size = '20px'; // can be 16 or 20 px
    const ammount = this.props.entitlementEntry.price;
    const symbol = this.props.currencyEntry.sign;
    const code = this.props.currencyEntry.code;
    const name = this.props.entitlementEntry.name;
    const isSecurityActive = this.props.entitlementEntry.applicability; //???
    return (
      <div style={EntitlementRow.STYLE.container}>
        <CheckMarkButton 
          size='20px'
          isReadOnly />
        <DropDownButton size='20px'/>
        <div style={EntitlementRow.STYLE.textBase}>{name}</div>
        <div style={EntitlementRow.STYLE.textBase}>
          {symbol}{ammount} {code}
        </div>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'flex-end' as 'flex-end'
    },
    buttonPadding: {
      padding: '4px'
    },
    textBase: {
      flexBasis: '0',
      font: 'Roboto',
      fontSize: '14px'
    },
    currencyBase: {
      justifyContent: 'flex-end' as 'flex-end'
    },
    name: {
      color:'#4B23A0'
    },
    greenCheckMark: {
      color: '#36BB55'
    },
    greyCheckMark: {
      color: '#000000'
    }
  };
}
