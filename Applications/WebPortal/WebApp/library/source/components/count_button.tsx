import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Button } from './button';

interface Properties extends
    Omit<React.ButtonHTMLAttributes<HTMLButtonElement>, 'children'> {

  /** The text to display on the button. */
  label: string;

  /** The count to display. Omit to hide the badge. */
  count?: number;
}

/** A secondary button with an optional count badge. */
export function CountButton(props: Properties): JSX.Element {
  const {label, count, className, ...attributes} = props;
  return <Button {...attributes} variant={Button.Variant.SECONDARY}
    className={[css(STYLES.button), className].join(' ')}
    label={<>
      <span>{label}</span>
      {count != null && <span className={css(STYLES.count)}>{count}</span>}
    </>}/>;
}

const STYLES = StyleSheet.create({
  button: {
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    gap: '8px',
    '--count-background': '#E8E8E8',
    ':enabled:hover': {'--count-background': '#C8C8C8'},
    ':enabled:focus-visible': {'--count-background': '#C8C8C8'},
    ':enabled:active': {'--count-background': '#C8C8C8'}
  },
  count: {
    flexShrink: 0,
    fontSize: '0.75rem',
    color: '#5D5E6D',
    textAlign: 'center',
    backgroundColor: 'var(--count-background)',
    padding: '2px 4px',
    borderRadius: '4px',
    userSelect: 'none'
  }
});
