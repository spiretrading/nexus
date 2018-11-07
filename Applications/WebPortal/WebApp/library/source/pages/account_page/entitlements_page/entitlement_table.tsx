import * as React from 'react';
import * as Nexus from 'nexus';
import {EntitlementsPageSizing} from './entitlements_page';

interface Properties {
  entitlementEntry?: Nexus.EntitlementDatabase.Entry;
  breakpoint?: EntitlementsPageSizing.BreakPoint;
}

export class EntitlementTable extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const buttonSize = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    return (
      <table style={EntitlementTable.STYLE.container}>
          <tr >
            <th style={EntitlementTable.STYLE.th}>Mkt</th>
            <th style={EntitlementTable.STYLE.th}>BBO</th>
            <th style={EntitlementTable.STYLE.th}>MQ</th>
            <th style={EntitlementTable.STYLE.th}>BQ</th>
            <th style={EntitlementTable.STYLE.th}>TnS</th>
            <th style={EntitlementTable.STYLE.th}>Imb</th>
        </tr>
        <tr>
          <th style={EntitlementTable.STYLE.th}>Market</th>
          <th style={EntitlementTable.STYLE.th}>BBO</th>
          <th style={EntitlementTable.STYLE.th}>Market Quotes</th>
          <th style={EntitlementTable.STYLE.th}>Book Quotes</th>
          <th style={EntitlementTable.STYLE.th}>Time n Sales</th>
          <th style={EntitlementTable.STYLE.th}>Imbalances</th>
       </tr>
       <tr>
          <td>XMLS</td>
          <td>
            <img style={EntitlementTable.STYLE.circle}
              src={'resources/account_page/entitlements_page/icons/dot-green.svg'}/>
          </td>
           <td>
            <img style={EntitlementTable.STYLE.circle}
              src={'resources/account_page/entitlements_page/icons/dot-green.svg'}/>
          </td>
           <td>
            <img style={EntitlementTable.STYLE.circle}
              src={'resources/account_page/entitlements_page/icons/dot-green.svg'}/>
          </td>
           <td>
            <img style={EntitlementTable.STYLE.circle}
              src={'resources/account_page/entitlements_page/icons/dot-green.svg'}/>
          </td>
           <td>
            <img style={EntitlementTable.STYLE.circle}
              src={'resources/account_page/entitlements_page/icons/dot-green.svg'}/>
          </td>
        </tr>
    </table>);
  }

  private static readonly STYLE = {
    container: {
      font: '14px Roboto',
      borderStyle: 'none' as 'none',
      cellspacing: 0,
      borderCollapse: 'collapse' as 'collapse',
      textAlign: 'center' as 'center'
    },
    circle: {
      height: '14px',
      width: '14px'
    },
    th: {
      backgroundColor: '#F8F8F8',
      fontWeight: 'normal' as 'normal'
    },
    td: {
    }
  };
}
