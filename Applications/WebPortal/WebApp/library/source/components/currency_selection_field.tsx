import * as Nexus from 'nexus';
import * as React from 'react';
import { Select } from './select';

interface Properties {

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The initial currency selected. */
  value?: Nexus.Currency;

  /** Additional CSS styles. */
  style?: React.CSSProperties;

  /** The class name of the currency selection box. */
  className?: string;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.Currency) => void;
}

/** A selection field for currencies. */
export function CurrencySelectionField(props: Properties): JSX.Element {
  const onChange = (value: string) => {
    props.onChange?.(props.currencyDatabase.fromCode(value).currency);
  };
  const defaultValue = props.value
    ? props.currencyDatabase.fromCurrency(props.value).code
    : undefined;
  const options = [];
  for(const currency of props.currencyDatabase) {
    options.push(
      <option value={currency.code} key={currency.code}>
        {currency.code}
      </option>);
  }
  return (
    <Select
      defaultValue={defaultValue}
      readonly={props.readonly}
      style={{...STYLE.base, ...props.style}}
      className={props.className}
      onChange={onChange}>
      {options}
    </Select>);
}

const STYLE = {
  base: {
    minWidth: '246px',
    width: '100%',
    flexGrow: 1,
    flexShrink: 1
  } as React.CSSProperties
};
