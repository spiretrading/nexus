import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The initial currency selected. */
  value?: Nexus.Currency;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the currency selection box. */
  className?: string;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.Currency) => void;
}

/** A selection field for currencies. */
export class CurrencySelectionField extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const style = (() => {
      if(this.props.readonly) {
        return CurrencySelectionField.STYLE.readonly;
      } else {
        return CurrencySelectionField.STYLE.selectionBox;
      }
    })();
    const defaultValue = (() => {
      if(this.props.value) {
        return this.props.currencyDatabase.fromCurrency(this.props.value).code;
      }
      return null;
    })();
    const currencyOptions = [];
    for(let currency of this.props.currencyDatabase) {
      currencyOptions.push(
        <option value={currency.code} key={currency.code}>
          {currency.code}
        </option>);
    }
    return (
      <select disabled={this.props.readonly}
          style={{...style, ...this.props.style}} 
          className={css(CurrencySelectionField.EXTRA_STYLE.effects) + ' ' +
            this.props.className}
          onChange={this.onChange} defaultValue={defaultValue}>
        {currencyOptions}
      </select>);
  }

  private onChange = (event: React.ChangeEvent<any>) => {
    if(!this.props.readonly) {
      this.props.onChange(this.props.currencyDatabase.fromCode(
        event.target.value).currency);
    }
  }

  private static readonly STYLE = {
    selectionBox: {
      boxSizing: 'border-box',
      height: '34px',
      font: '400 14px Roboto',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage: 'url(resources/components/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none',
      WebkitAppearance: 'none',
      appearance: 'none',
      minWidth: '246px',
      width: '100%',
      flexGrow: 1,
      flexShrink: 1,
      cursor: 'pointer'
    } as React.CSSProperties,
    readonly: {
      boxSizing: 'border-box',
      height: '34px',
      font: '400 14px Roboto',
      paddingLeft: '7px',
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#FFFFFF',
      backgroundImage: 'url(resources/components/arrow-down-grey.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none',
      WebkitAppearance: 'none',
      appearance: 'none',
      minWidth: '246px',
      width: '100%',
      flexGrow: 1,
      flexShrink: 1
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        outlineColor: 'transparent',
        outlineStyle: 'none',
        border: '1px solid #684BC7',
        borderRadius: '1px'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      },
      '-webkit-user-select': 'text',
      '-moz-user-select': 'text',
      '-ms-user-select': 'text',
      'user-select': 'text'
    }
  });
}
