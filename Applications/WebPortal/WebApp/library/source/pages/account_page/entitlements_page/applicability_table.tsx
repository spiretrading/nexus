import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** The entitlement the table belongs to. */
  entitlementEntry: Nexus.EntitlementDatabase.Entry;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

/*Displays a applicability table */
export class ApplicabilityTable extends React.Component<Properties, {}> {
  constructor(properties: Properties) {
    super(properties);
    this.getDisplayName = this.getDisplayName.bind(this);
  }

  public render(): JSX.Element {
    const containerWidth = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ApplicabilityTable.STYLE.container.small;
      } else {
        return ApplicabilityTable.STYLE.container.notSmall;
      }
    })();
    const compactHeader = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ApplicabilityTable.STYLE.headerLabel;
      } else {
        return ApplicabilityTable.STYLE.hiddenHeader;
      }
    })();
    const expandedHeader = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ApplicabilityTable.STYLE.hiddenHeader;
      } else {
        return ApplicabilityTable.STYLE.headerLabel;
      }
    })();
    const tableData = (() => {
      const data = [];
      const MarketDataTypes = [
        Nexus.MarketDataType.BBO_QUOTE,
        Nexus.MarketDataType.BOOK_QUOTE,
        Nexus.MarketDataType.TIME_AND_SALE,
        Nexus.MarketDataType.ORDER_IMBALANCE];
      for(const app of this.props.entitlementEntry.applicability) {
        const dots = [];
        for(const type of MarketDataTypes) {
          let imageSrc;
          if(app[1].test(type)) {
            imageSrc =
              'resources/account_page/entitlements_page/icons/dot-green.svg';
          } else {
            imageSrc =
              'resources/account_page/entitlements_page/icons/dot-grey.svg';
          }
          dots.push(
            <td key={type.toString()}>
              <img style={ApplicabilityTable.STYLE.circle} src={imageSrc}/>
            </td>);
        }
        data.push(
          <tr key={app[0].source.toString()}
              style={ApplicabilityTable.STYLE.row}>
            <td>{this.getDisplayName(app[0].source)}</td>{dots}
          </tr>);
      }
      return data;
    })();
    return (
      <table style={{...ApplicabilityTable.STYLE.container,...containerWidth}}>
        <thead>
          <tr>
            <th style={{...compactHeader,
                ...ApplicabilityTable.STYLE.marketWidth}}>
              Mkt
            </th>
            <th style={compactHeader}>BBO</th>
            <th style={compactHeader}>BQ</th>
            <th style={compactHeader}>T{'&'}S</th>
            <th style={compactHeader}>Imb</th>
          </tr>
          <tr>
            <th style={expandedHeader}>Market</th>
            <th style={expandedHeader}>BBO</th>
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

  private getDisplayName(venue: Nexus.Venue) {
    return this.props.venueDatabase.fromVenue(venue).displayName;
  }

  private static readonly STYLE = {
    container: {
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      cellspacing: 0,
      borderSpacing: 0,
      textAlign: 'center' as 'center',
      padding: 0,
      small: {
        minWidth: '284px',
        width: 'inherit' as 'inherit',
        maxWidth: '424px'
      },
      notSmall: {
        width: '636px'
      }
    },
    headerLabel: {
      height: '40px',
      backgroundColor: '#F8F8F8',
      borderRadius: '1px',
      fontWeight: 400,
      padding: 0,
      cellspacing: 0,
      borderCollapse: 'collapse' as 'collapse',
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
