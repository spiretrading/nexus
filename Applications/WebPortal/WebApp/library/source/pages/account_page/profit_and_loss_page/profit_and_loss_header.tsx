import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { CurrencyTooltip } from './currency_tooltip';
import { Metric } from './metric';

interface Properties {

  /** The currency symbol (e.g. "$"). */
  symbol: string;

  /** The currency code (e.g. "CAD"). */
  code: string;

  /** The total profit and loss. */
  totalPnl: string;

  /** The total fees incurred. */
  totalFees: string;

  /** The total volume traded. */
  totalVolume: string;

  /** The exchange rates for foreign currencies. */
  foreignCurrencies: CurrencyTooltip.ExchangeRate[];

  /** Whether the header is in a loading state. */
  loading?: boolean;
}

/** Displays summary statistics for the profit and loss report. */
export function ProfitAndLossHeader(props: Properties) {
  const [isTooltipShown, setIsTooltipShown] = React.useState(false);
  const pnlValue = (() => {
    const num = parseFloat(props.totalPnl);
    if(num < 0) {
      return `-${props.symbol}${Math.abs(num)}`;
    }
    return `${props.symbol}${props.totalPnl}`;
  })();
  const showInfo = props.foreignCurrencies.length > 0 && !props.loading;
  return (
    <div className={css(STYLES.wrapper)}>
      <header className={css(STYLES.header)}
          aria-live='polite'
          aria-busy={props.loading ? 'true' : 'false'}
          aria-label='Profit and Loss Report Summary Statistics'>
        <div className={css(STYLES.summaryGroup)}>
          <Metric id='total-pnl' label='Total P/L'
            value={pnlValue} unit={props.code} loading={props.loading}/>
          <Metric id='total-fees' label='Fees'
            value={`${props.symbol}${props.totalFees}`} unit={props.code}
            loading={props.loading}/>
          <Metric id='total-volume' label='Volume'
            value={`${props.symbol}${props.totalVolume}`}
            loading={props.loading}/>
        </div>
        {showInfo &&
          <div className={css(STYLES.infoWrapper)}
              onMouseEnter={() => setIsTooltipShown(true)}
              onMouseLeave={() => setIsTooltipShown(false)}>
            <img
              src='resources/account_page/profit_and_loss_page/info.svg'
              width='16' height='16'
              aria-label='Currency exchange information'/>
            {isTooltipShown &&
              <div className={css(STYLES.tooltipAnchor)}>
                <CurrencyTooltip accountCurrency={props.code}
                  exchangeRates={props.foreignCurrencies}/>
              </div>}
          </div>}
      </header>
    </div>);
}

const STYLES = StyleSheet.create({
  wrapper: {
    containerType: 'inline-size'
  },
  header: {
    display: 'flex',
    alignItems: 'flex-start',
    gap: '8px',
    backgroundColor: '#F8F8F8',
    borderRadius: '8px',
    padding: '18px 12px',
    '@container (min-width: 732px)': {
      backgroundColor: 'transparent',
      borderRadius: 0,
      padding: 0
    }
  },
  summaryGroup: {
    display: 'flex',
    alignItems: 'center',
    flexFlow: 'row wrap',
    gap: '10px',
    justifyContent: 'space-between',
    flex: 1,
    '@container (min-width: 732px)': {
      justifyContent: 'start',
      gap: '30px',
      flex: 'none'
    }
  },
  infoWrapper: {
    position: 'relative',
    flexShrink: 0,
    cursor: 'pointer',
    lineHeight: 0
  },
  tooltipAnchor: {
    position: 'absolute',
    top: '100%',
    right: 0,
    zIndex: 1,
    paddingTop: '4px',
    lineHeight: 'normal'
  }
});
