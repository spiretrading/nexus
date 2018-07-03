import * as React from 'react';
import * as Nexus from 'nexus';

/** The React props used to render the CurrencySelectionBox. */
interface Properties {

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The initial currency selected. */
  value?: Nexus.Currency;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.Currency) => void;
}

/** Displays a selection box for currencies. */
export class CurrencySelectionBox extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const selectStyle = {
      boxSizing: 'border-box',
      width: '100%',
      height: '34px',
      font: '16px Roboto',
      border: '1px solid #C8C8C8',
      appearance: 'none',
      WebkitAppearance: 'none',
      MozAppearance: 'none',
      background: `url("data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='10' height='6' stroke='black' fill='none'><polyline points='0,0 4.5,5 9,0'/></svg>")`,
      backgroundWidth: '10px',
      backgroundHeight: '6px',
      backgroundPosition: 'calc(100% - 10px) center',
      backgroundRepeat: 'no-repeat',
      paddingLeft: '6px',
      paddingRight: '6px',
      cursor: 'pointer'
    } as any;
    const currencies = [];
    for(let currency of this.props.currencyDatabase) {
      currencies.push(
        <option value={currency.code} key={currency.code}>
          {currency.code}
        </option>);
    }
    const defaultValue = (() => {
      if(this.props.value) {
        return this.props.currencyDatabase.fromCurrency(this.props.value).code;
      }
      return undefined;
    })();
    return (
      <div>
        <select style={selectStyle} onChange={this.onChange}
            defaultValue={defaultValue}>
          {currencies}
        </select>
      </div>);
  }

  private onChange(event: React.ChangeEvent<any>): void {
    if(this.props.onChange) {
      this.props.onChange(this.props.currencyDatabase.fromCode(
        event.target.value).currency);
    }
  }
}
