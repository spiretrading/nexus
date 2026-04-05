import * as Nexus from 'nexus';
import * as React from 'react';
import { DecimalInput } from './decimal_input';

interface Properties extends
    Omit<React.ComponentProps<typeof DecimalInput>,
      'min' | 'max' | 'value' | 'onChange'> {

  /** The minimum allowed value (inclusive). */
  min?: Nexus.Money;

  /** The maximum allowed value (inclusive). */
  max?: Nexus.Money;

  /** The value to display. */
  value?: Nexus.Money;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value?: Nexus.Money) => void;
}

/** An editable money input. */
export function MoneyInput({min, max, value, onChange,
    decimalPlaces = 2, ...rest}: Properties): JSX.Element {
  const onDecimalChange = (v?: number) => {
    onChange?.(v != null ? Nexus.Money.parse(v.toString()) : undefined);
  };
  return (
    <DecimalInput
      {...rest}
      decimalPlaces={decimalPlaces}
      min={min != null ? toNumber(min) : undefined}
      max={max != null ? toNumber(max) : undefined}
      value={value != null ? toNumber(value) : undefined}
      onChange={onDecimalChange}/>);
}

type MoneyLabelProperties = Omit<Properties, 'readOnly' | 'onChange'>;

/** A read-only money display. */
export function MoneyLabel(props: MoneyLabelProperties): JSX.Element {
  return <MoneyInput {...props} readOnly/>;
}

function toNumber(money: Nexus.Money): number {
  return parseFloat(money.toString());
}
