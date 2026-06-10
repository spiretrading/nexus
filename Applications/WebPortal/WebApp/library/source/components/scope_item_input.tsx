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
   * @param value - The scope.
   */
  onEnter?: (value: Nexus.Scope) => void;
}

/** Renders an input field for a single scope item. */
export function ScopeItemInput({onChange, onEnter, ...rest}: Properties):
    JSX.Element {
  const onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange?.(event.target.value);
  };
  const onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'Enter') {
      const scope = parseScopeItem(String(rest.value ?? ''));
      if(scope) {
        onEnter?.(scope);
      }
    }
  };
  return (
    <Input
      placeholder='Scope'
      {...rest}
      onChange={onInputChange}
      onKeyDown={onKeyDown}/>);
}

/** Parses a single line of text into a scope item.
 * @param text - The text to parse.
 * @return The parsed Scope, or null if the text could not be parsed.
 */
export function parseScopeItem(text: string): Nexus.Scope {
  const name = text.trim().toUpperCase();
  if(name === '*') {
    return Nexus.Scope.makeGlobal('*');
  }
  const country = parseCountry(name);
  if(!country.equals(Nexus.CountryCode.NONE)) {
    return new Nexus.Scope(country);
  }
  const venue = parseVenue(name);
  if(!venue.equals(Nexus.Venue.NONE)) {
    return new Nexus.Scope(venue);
  }
  const ticker = Nexus.Ticker.parse(name);
  if(!ticker.equals(Nexus.Ticker.NONE)) {
    return new Nexus.Scope(ticker);
  }
  return null;
}

function parseCountry(text: string): Nexus.CountryCode {
  for(const country of Nexus.countryDatabase) {
    if(text === country.twoLetterCode || text === country.threeLetterCode ||
        text === country.name.toUpperCase()) {
      return country.code;
    }
  }
  return Nexus.CountryCode.NONE;
}

function parseVenue(text: string): Nexus.Venue {
  for(const venue of Nexus.venueDatabase) {
    if(text === venue.displayName.toUpperCase() ||
        text === venue.venue.toString()) {
      return venue.venue;
    }
  }
  return Nexus.Venue.NONE;
}
