import * as Nexus from 'nexus';
import * as React from 'react';
import { Input } from './input';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>, 'onChange'> {

  /** Called when the displayed value changes.
   * @param value - The new value.
   */
  onChange?: (value: string) => void;

  /** Called when the value is submitted.
   * @param value - The region.
   */
  onEnter?: (value: Nexus.Region) => void;
}

/** Renders an input field for a single region item. */
export function RegionItemInput({onChange, onEnter, ...rest}:
    Properties): JSX.Element {
  const onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange?.(event.target.value);
  };
  const onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'Enter') {
      const name = String(rest.value ?? '').trim().toUpperCase();
      if(name === '*') {
        onEnter?.(Nexus.Region.makeGlobal('*'));
        return;
      }
      const country = parseCountry(name);
      if(!country.equals(Nexus.CountryCode.NONE)) {
        onEnter?.(new Nexus.Region(country));
        return;
      }
      const venue = parseVenue(name);
      if(!venue.equals(Nexus.Venue.NONE)) {
        onEnter?.(new Nexus.Region(venue));
        return;
      }
      const ticker = Nexus.Ticker.parse(name);
      if(!ticker.equals(Nexus.Ticker.NONE)) {
        onEnter?.(new Nexus.Region(ticker));
        return;
      }
    }
  };
  return (
    <Input
      placeholder='Region'
      {...rest}
      onChange={onInputChange}
      onKeyDown={onKeyDown}/>);
}


function parseCountry(text: string): Nexus.CountryCode {
  for(const country of Nexus.defaultCountryDatabase) {
    if(text === country.twoLetterCode || text === country.threeLetterCode ||
        text === country.name.toUpperCase()) {
      return country.code;
    }
  }
  return Nexus.CountryCode.NONE;
}

function parseVenue(text: string): Nexus.Venue {
  for(const venue of Nexus.defaultVenueDatabase) {
    if(text === venue.displayName.toUpperCase() ||
        text === venue.venue.toString()) {
      return venue.venue;
    }
  }
  return Nexus.Venue.NONE;
}
