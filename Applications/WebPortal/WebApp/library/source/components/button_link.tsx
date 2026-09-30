import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.AnchorHTMLAttributes<HTMLAnchorElement>, 'children'> {

  /** The text displayed by the link. */
  label: string;

  /** Whether the link is unavailable for user interaction. */
  inert?: boolean;
}

/** A native hyperlink styled as a button. */
export function ButtonLink(props: Properties): JSX.Element {
  const {label, inert, className, ...rest} = props;
  const attributes: {inert?: string} = {};
  if(inert) {
    attributes.inert = '';
  }
  const link = <a {...rest} {...attributes}
      className={[css(STYLES.link), className].join(' ')}>
    {label}
  </a>;
  if(inert) {
    return <span className={css(STYLES.inert)}>{link}</span>;
  }
  return link;
}

const STYLES = StyleSheet.create({
  link: {
    display: 'inline-block',
    boxSizing: 'border-box',
    backgroundColor: '#684BC7',
    color: '#FFFFFF',
    border: '1px solid transparent',
    borderRadius: '1px',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontSize: '0.875rem',
    fontWeight: 400,
    padding: '3px 9px',
    cursor: 'pointer',
    outline: 'none',
    textDecoration: 'none',
    ':hover': {backgroundColor: '#4B23A0'},
    ':focus-visible': {backgroundColor: '#4B23A0'},
    ':active': {backgroundColor: '#4B23A0'},
    ':is([inert])': {opacity: 0.4, cursor: 'not-allowed'}
  },
  inert: {display: 'inline-block', cursor: 'not-allowed'}
});
