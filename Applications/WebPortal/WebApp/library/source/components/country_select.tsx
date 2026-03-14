import * as Nexus from 'nexus';
import * as React from 'react';
import { Select } from './select';

interface Properties extends
    Omit<React.ComponentProps<typeof Select>, 'value' | 'onChange'> {

  /** The set of available countries to select. */
  countryDatabase: Nexus.CountryDatabase;

  /** The currently selected country. */
  value: Nexus.CountryCode;

  /** The event handler called when the selection changes. */
  onChange?: (country: Nexus.CountryCode) => void;
}

/** A country selection component. */
export function CountrySelect({countryDatabase, value, onChange, ...rest}:
    Properties): JSX.Element {
  const onSelectChange = (v: string) => {
    onChange?.(new Nexus.CountryCode(parseInt(v, 10)));
  };
  const options = [];
  for(const country of countryDatabase) {
    options.push(
      <option value={country.code.code} key={country.code.code}>
        {country.name}
      </option>);
  }
  return (
    <Select {...rest} value={value.code.toString()}
        onChange={onSelectChange}>
      {options}
    </Select>);
}

type CountryLabelProperties = Omit<Properties, 'readOnly' | 'onChange'>;

/** A read-only country display. */
export function CountryLabel(props: CountryLabelProperties): JSX.Element {
  return <CountrySelect {...props} readOnly/>;
}
