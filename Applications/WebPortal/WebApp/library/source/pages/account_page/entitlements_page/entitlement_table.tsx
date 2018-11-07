import * as React from 'react';
import * as Nexus from 'nexus';
import * as Beam from 'beam';
import {EntitlementsPageSizing} from './entitlements_page';

interface Properties {
  entitlementEntry: Nexus.EntitlementDatabase.Entry;
  breakpoint: EntitlementsPageSizing.BreakPoint;
}

export class EntitlementTable extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const compactHeader = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
       return EntitlementTable.STYLE.header;
      } else {
        return EntitlementTable.STYLE.hiddenHeader;
      }
    })();
    const expandedHeader = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementTable.STYLE.hiddenHeader;
      } else {
        return EntitlementTable.STYLE.header;
      }
    })();
    const containerWidth = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementTable.STYLE.container.small;
      } else {
        return EntitlementTable.STYLE.container.notSmall;
      }
    })();
    const greysrc =
      'resources/account_page/entitlements_page/icons/dot-grey.svg';
    const grensrc =
      'resources/account_page/entitlements_page/icons/dot-green.svg';
    const tableData = (() => {
      const data = [];
      for(const applicability of this.props.entitlementEntry.applicability) {
        const dots = [];
        const green =
          (<td>
            <img src={grensrc} style={EntitlementTable.STYLE.circle}/>
          </td>);
        const grey =
          (<td>
            <img src={greysrc} style={EntitlementTable.STYLE.circle}/>
          </td>);
        if(applicability[1].test(Nexus.MarketDataType.BBO_QUOTE)) {
          dots.push(green);
        } else {
          dots.push(grey);
        }
        if(applicability[1].test(Nexus.MarketDataType.MARKET_QUOTE)) {
          dots.push(green);
        } else {
          dots.push(grey);
        }
        if(applicability[1].test(Nexus.MarketDataType.BOOK_QUOTE)) {
          dots.push(green);
        } else {
          dots.push(grey);
        }
        if(applicability[1].test(Nexus.MarketDataType.TIME_AND_SALE)) {
          dots.push(green);
        } else {
          dots.push(grey);
        }
        if(applicability[1].test(Nexus.MarketDataType.ORDER_IMBALANCE)) {
          dots.push(green);
        } else {
          dots.push(grey);
        }
        data.push(
          <tr style={EntitlementTable.STYLE.row}>
            <td>{applicability[0].source.toString()}</td>
            {dots}
          </tr>);
      }
      return data;
    })();
    return (
      <div>
      <table style={{...EntitlementTable.STYLE.container,...containerWidth}}>
          <thead>
              <tr>
                <th style={compactHeader}>Mkt</th>
                <th style={compactHeader}>BBO</th>
                <th style={compactHeader}>MQ</th>
                <th style={compactHeader}>BQ</th>
                <th style={compactHeader}>T{'&'}S</th>
                <th style={compactHeader}>Imb</th>
            </tr>
            <tr>
              <th style={expandedHeader}>Market</th>
              <th style={expandedHeader}>BBO</th>
              <th style={expandedHeader}>Market Quotes</th>
              <th style={expandedHeader}>Book Quotes</th>
              <th style={expandedHeader}>Time {'&'} Sales</th>
              <th style={expandedHeader}>Imbalances</th>
          </tr>
          </thead>
          <tbody>
            {tableData}
          </tbody>
        </table>
        </div>);
  }

  private static readonly STYLE = {
    container: {
      font: '400 14px Roboto',
      borderStyle: 'none' as 'none',
      cellspacing: 0,
      borderCollapse: 'collapse' as 'collapse',
      textAlign: 'center' as 'center',
      small: {
        minWidth: '284px',
        maxWidth: '242px'
      },
      notSmall: {
        width: '636px'
      }
    },
    row: {
      height: '40px'
    },
    header: {
      height: '40px',
      fontWeight: 400,
      backgroundColor: '#F8F8F8'
    },
    hiddenHeader: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none',
      height: '0px'
    },
    circle: {
      height: '14px',
      width: '14px'
    }
  };
}
