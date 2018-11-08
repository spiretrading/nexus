import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import {EntitlementsPageSizing} from './entitlements_page';

interface Properties {
  marketDatabase: Nexus.MarketDatabase;
  entitlementEntry: Nexus.EntitlementDatabase.Entry;
  breakpoint: EntitlementsPageSizing.BreakPoint;
}

export class EntitlementTable extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
    this.getDisplayName = this.getDisplayName.bind(this);
  }

  public render(): JSX.Element {
    const containerWidth = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementTable.STYLE.container.small;
      } else {
        return EntitlementTable.STYLE.container.notSmall;
      }
    })();
    const compactHeader = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementTable.STYLE.headerLabel;
      } else {
        return EntitlementTable.STYLE.hiddenHeader;
      }
    })();
    const expandedHeader = (() => {
      if(this.props.breakpoint === EntitlementsPageSizing.BreakPoint.SMALL) {
        return EntitlementTable.STYLE.hiddenHeader;
      } else {
        return EntitlementTable.STYLE.headerLabel;
      }
    })();
    const tableData = (() => {
      const data = [];
      for(const applicability of this.props.entitlementEntry.applicability) {
        const dots = [];
        const greenCircle =
          (<td>
            <img style={EntitlementTable.STYLE.circle}
              src=
                'resources/account_page/entitlements_page/icons/dot-green.svg'/>
          </td>);
        const greyCircle =
          (<td>
            <img style={EntitlementTable.STYLE.circle}
              src=
                'resources/account_page/entitlements_page/icons/dot-grey.svg'/>
          </td>);
        if(applicability[1].test(Nexus.MarketDataType.BBO_QUOTE)) {
          dots.push(greenCircle);
        } else {
          dots.push(greyCircle);
        }
        if(applicability[1].test(Nexus.MarketDataType.MARKET_QUOTE)) {
          dots.push(greenCircle);
        } else {
          dots.push(greyCircle);
        }
        if(applicability[1].test(Nexus.MarketDataType.BOOK_QUOTE)) {
          dots.push(greenCircle);
        } else {
          dots.push(greyCircle);
        }
        if(applicability[1].test(Nexus.MarketDataType.TIME_AND_SALE)) {
          dots.push(greenCircle);
        } else {
          dots.push(greyCircle);
        }
        if(applicability[1].test(Nexus.MarketDataType.ORDER_IMBALANCE)) {
          dots.push(greenCircle);
        } else {
          dots.push(greyCircle);
        }
        data.push(
          <tr style={EntitlementTable.STYLE.row}>
            <td>{this.getDisplayName(applicability[0].source)}</td>{dots}
          </tr>);
      }
      return data;
    })();
    return (
      <table style={{...EntitlementTable.STYLE.container,...containerWidth}}>
        <thead>
          <tr>
            <th style={{...compactHeader,
                ...EntitlementTable.STYLE.marketWidth}}>
              Mkt
            </th>
            <th style={compactHeader}>BBO</th>
            <th style={compactHeader}>MQ</th>
            <th style={compactHeader}>BQ</th>
            <th style={compactHeader}>T{'&'}S</th>
            <th style={compactHeader}>Imb</th>
          </tr>
          <tr>
            <th style={{...expandedHeader}}>Market</th>
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
      </table>);
  }

  private getDisplayName(code: Nexus.MarketCode) {
    const market = this.props.marketDatabase.fromCode(code);
    return market.displayName;
  }

  private static readonly STYLE = {
    container: {
      border: '1px solid #C8C8C8',
      font: '400 14px Roboto',
      borderCollapse: 'collapse' as 'collapse',
      textAlign: 'center' as 'center',
      padding: 0,
      small: {
        minWidth: '284px',
        maxWidth: '242px'
      },
      notSmall: {
        width: '636px'
      }
    },
    headerLabel: {
      height: '40px',
      backgroundColor: '#F8F8F8',
      borderTop: '1px solid #C8C8C8',
      fontWeight: 400,
      padding: 0,
      marginLeft: '19px',
      marginRight: '19px'
    },
    marketWidth: {
      width: '22%'
    },
    hiddenHeader: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none',
      height: '0px'
    },
    row: {
      height: '40px'
    },
    circle: {
      height: '14px',
      width: '14px'
    }
  };
}
