import * as Nexus from 'nexus';
import * as React from 'react';
import { Select } from './select';

interface Properties extends
    Omit<React.ComponentProps<typeof Select>, 'value' | 'onChange'> {

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The currently selected currency. */
  value?: Nexus.Currency;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.Currency) => void;
}

/** A currency selection component. */
export function CurrencySelect({currencyDatabase, value, onChange, ...rest}:
    Properties): JSX.Element {
  const onSelectChange = (v: string) => {
    onChange?.(currencyDatabase.fromCode(v).currency);
  };
  const selectValue = value
    ? currencyDatabase.fromCurrency(value).code
    : undefined;
  const options = [];
  for(const currency of currencyDatabase) {
    options.push(
      <option value={currency.code} key={currency.code}>
        {currency.code}
      </option>);
  }
  return (
    <Select {...rest} value={selectValue} onChange={onSelectChange}>
      {options}
    </Select>);
}
