import * as React from 'react';

interface Properties {

  /** The account's primary currency code (e.g. "CAD"). */
  accountCurrency: string;

  /** The exchange rates keyed by currency code. The value represents how much
   *  1 unit of the currency is worth in account_currency. */
  exchangeRates: CurrencyTooltip.ExchangeRate[];
}

/** Displays a tooltip listing exchange rates for foreign currencies. */
export function CurrencyTooltip(props: Properties) {
  return (
    <div style={STYLE.container}>
      <div style={STYLE.tooltip}>
        <ul style={STYLE.list}>
          {props.exchangeRates.map((entry, i) =>
            <li key={entry.code} style={{...STYLE.item,
                ...(i === props.exchangeRates.length - 1 &&
                  STYLE.lastItem)}}>
              <span>{`1 ${entry.code}`}</span>
              <span style={STYLE.rate}>
                {`${entry.rate} ${props.accountCurrency}`}
              </span>
            </li>)}
        </ul>
      </div>
    </div>);
}

export namespace CurrencyTooltip {
  export interface ExchangeRate {

    /** The currency code (e.g. "AUD"). */
    code: string;

    /** The exchange rate in terms of the account currency. */
    rate: string;
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  container: {
    width: 'min(200px, 20ch)'
  },
  tooltip: {
    fontSize: '0.875rem',
    color: '#333333',
    backgroundColor: '#FFFFFF',
    borderRadius: '1px',
    boxShadow: '0 0 2px rgb(0 0 0 / 40%)',
    padding: '5px 15px',
    transition: 'opacity 100ms ease'
  },
  list: {
    padding: 0,
    margin: 0,
    listStyle: 'none'
  },
  item: {
    display: 'flex',
    justifyContent: 'space-between',
    gap: '18px',
    padding: '3px 0 2px',
    borderBottom: '1px solid #E6E6E6'
  },
  lastItem: {
    borderColor: 'transparent'
  },
  rate: {
    textAlign: 'end'
  }
};
