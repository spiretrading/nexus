import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends React.AnchorHTMLAttributes<HTMLAnchorElement> {

  /** The text to display. */
  label: string;
}

/** A styled anchor link. */
export function Link(props: Properties): JSX.Element {
  return (
    <a href={props.href}
        target={props.target}
        className={[css(STYLES.link), props.className].join(' ')}
        style={props.style}
        onClick={props.onClick}>
      {props.label}
    </a>);
}

const STYLES = StyleSheet.create({
  link: {
    fontSize: '0.875rem',
    color: '#007AE6',
    textDecoration: 'none',
    ':hover': {
      color: '#684BC7'
    },
    ':active': {
      color: '#684BC7'
    },
    ':focus-visible': {
      outlineOffset: '2px',
      outline: '1px solid #684BC7'
    }
  }
});
