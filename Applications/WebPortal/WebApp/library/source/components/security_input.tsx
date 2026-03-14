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
   * @param value - The security.
   */
  onEnter?: (value: Nexus.Security) => void;
}

/** The component that uses user input to get a security. */
export function SecurityInput({onChange, onEnter, ...rest}:
    Properties): JSX.Element {
  const onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange?.(event.target.value.toUpperCase());
  };
  const onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'Enter') {
      const security = Nexus.Security.parse(String(rest.value ?? ''));
      if(!security.equals(Nexus.Security.NONE)) {
        onEnter?.(security);
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

