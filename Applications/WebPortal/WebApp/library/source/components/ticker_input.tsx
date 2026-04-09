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
   * @param value - The ticker.
   */
  onEnter?: (value: Nexus.Ticker) => void;
}

/** The component that uses user input to get a ticker. */
export function TickerInput({onChange, onEnter, ...rest}: Properties):
    JSX.Element {
  const onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange?.(event.target.value.toUpperCase());
  };
  const onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'Enter') {
      const ticker = Nexus.Ticker.parse(String(rest.value ?? ''));
      if(!ticker.equals(Nexus.Ticker.NONE)) {
        onEnter?.(ticker);
      }
    }
  };
  return (
    <Input
      placeholder='Find symbol here'
      {...rest}
      onChange={onInputChange}
      onKeyDown={onKeyDown}/>);
}
