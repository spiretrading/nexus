import * as React from 'react';
import * as Nexus from 'nexus';
import { DisplaySize } from '..';
import { Select } from './select';
import { TextField } from './text_field';

interface Properties {

  /** The set of available countries to select. */
  countryDatabase: Nexus.CountryDatabase;

  /** The currently selected country. */
  value: Nexus.CountryCode;

  /** Whether the selection box is read only. */
  readonly?: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (country: Nexus.CountryCode) => void;

  /** Determines the size of the element. */
  displaySize: DisplaySize;
}

/** A country selection field. */
export function CountrySelect(props: Properties): JSX.Element {
  const onChange = (value: string) => {
    props.onChange?.(new Nexus.CountryCode(parseInt(value, 10)));
  };
  if(props.readonly) {
    return (
      <TextField
        value={props.countryDatabase.fromCode(props.value).name}
        displaySize={props.displaySize}/>);
  }
  const style = props.displaySize === DisplaySize.SMALL ?
    STYLE.boxSmall : STYLE.boxLarge;
  const options = [];
  for(const country of props.countryDatabase) {
    options.push(
      <option value={country.code.code} key={country.code.code}>
        {country.name}
      </option>);
  }
  return (
    <Select value={props.value.code.toString()} style={style}
        onChange={onChange}>
      {options}
    </Select>);
}

const STYLE = {
  boxSmall: {
    font: '400 16px Roboto',
    width: '100%'
  } as React.CSSProperties,
  boxLarge: {
    width: '200px',
    font: '400 14px Roboto'
  } as React.CSSProperties
};
