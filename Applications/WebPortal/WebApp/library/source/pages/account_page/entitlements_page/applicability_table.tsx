import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** The entitlement the table belongs to. */
  entitlementEntry: Nexus.EntitlementDatabase.Entry;
}

const MARKET_DATA_TYPES = [
  Nexus.MarketDataType.BBO_QUOTE,
  Nexus.MarketDataType.BOOK_QUOTE,
  Nexus.MarketDataType.TIME_AND_SALE,
  Nexus.MarketDataType.ORDER_IMBALANCE];

/** Displays an applicability table. */
export function ApplicabilityTable(props: Properties): JSX.Element {
  const rows = [];
  for(const app of props.entitlementEntry.applicability) {
    const dots = MARKET_DATA_TYPES.map(type => {
      const granted = app[1].test(type);
      const src = granted ?
        'resources/account_page/entitlements_page/icons/dot-green.svg' :
        'resources/account_page/entitlements_page/icons/dot-grey.svg';
      return (
        <td key={type.toString()} className={css(STYLES.cell)}>
          <img style={STYLE.circle} src={src}
            aria-label={granted ? 'true' : 'false'}/>
        </td>);
    });
    rows.push(
      <tr key={app[0].source.toString()} style={STYLE.row}>
        <td className={css(STYLES.cell)} style={STYLE.venueCell}>
          {props.venueDatabase.fromVenue(app[0].source).displayName}
        </td>
        {dots}
      </tr>);
  }
  return (
    <table className={css(STYLES.container)}>
      <thead>
        <tr className={css(STYLES.compactHeader)}>
          <td className={css(STYLES.cell)} style={STYLE.venueCell}>Venue</td>
          <td className={css(STYLES.cell)}>BBO</td>
          <td className={css(STYLES.cell)}>BQ</td>
          <td className={css(STYLES.cell)}>T{'&'}S</td>
          <td className={css(STYLES.cell)}>Imb</td>
        </tr>
        <tr className={css(STYLES.expandedHeader)}>
          <td className={css(STYLES.cell)} style={STYLE.venueCell}>Venue</td>
          <td className={css(STYLES.cell)}>BBO</td>
          <td className={css(STYLES.cell)}>Book Quotes</td>
          <td className={css(STYLES.cell)}>Time {'&'} Sales</td>
          <td className={css(STYLES.cell)}>Imbalances</td>
        </tr>
      </thead>
      <tbody>
        {rows}
      </tbody>
    </table>);
}

const STYLE: Record<string, React.CSSProperties> = {
  circle: {
    height: '14px',
    width: '14px'
  },
  row: {
    height: '40px'
  },
  venueCell: {
    width: '22%',
    textAlign: 'start'
  }
};

const STYLES = StyleSheet.create({
  container: {
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    borderSpacing: 0,
    textAlign: 'center',
    padding: 0,
    width: '100%',
    '@media (min-width: 768px)': {
      width: '636px'
    }
  },
  compactHeader: {
    height: '40px',
    backgroundColor: '#F8F8F8',
    borderRadius: '1px',
    fontWeight: 500,
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  expandedHeader: {
    display: 'none',
    height: '40px',
    backgroundColor: '#F8F8F8',
    borderRadius: '1px',
    fontWeight: 500,
    '@media (min-width: 768px)': {
      display: 'table-row'
    }
  },
  cell: {
    padding: '12px 10px',
    '@media (min-width: 768px)': {
      padding: '12px 18px'
    }
  }
});
