import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The account to link to. */
  account: Beam.DirectoryEntry;

  /** The display variant. Defaults to NAME. */
  variant?: AccountLink.Variant;

  /** Initials rendered in the avatar. Required when variant is AVATAR. */
  initials?: string;

  /** A color used as a fallback when no avatar is available. Required when
   *  variant is AVATAR. */
  tint?: string;

  /** Called when the link is clicked, passing the account. */
  onClick?: (account: Beam.DirectoryEntry) => void;
}

/** Displays a link to an account with an optional avatar. */
export function AccountLink(props: Properties) {
  const variant = props.variant ?? AccountLink.Variant.NAME;
  const onClick = (event: React.MouseEvent) => {
    event.preventDefault();
    props.onClick?.(props.account);
  };
  return (
    <a className={css(STYLES.link)}
        href={`account/${props.account.id}/profile`}
        onClick={onClick}>
      {variant === AccountLink.Variant.AVATAR && renderAvatar(props)}
      <span>{props.account.name}</span>
    </a>);
}

function renderAvatar(props: Properties): JSX.Element {
  const dynamicStyle = StyleSheet.create({
    avatar: {
      backgroundColor: props.tint,
      boxShadow: `0 0 0 2px ${props.tint}`
    }
  });
  return (
    <span
        aria-hidden='true' className={css(STYLES.avatar, dynamicStyle.avatar)}>
      {props.initials}
    </span>);
}

export namespace AccountLink {

  /** Enumerates the display variants of an AccountLink. */
  export enum Variant {

    /** Displays only the account name. */
    NAME,

    /** Displays an avatar circle beside the account name. */
    AVATAR
  }
}

const STYLES = StyleSheet.create({
  link: {
    display: 'inline-flex',
    alignItems: 'center' as 'center',
    gap: '6px',
    padding: '8px 0',
    textDecoration: 'none',
    color: '#333333',
    fontSize: '0.875rem',
    fontFamily: 'Roboto',
    outlineOffset: '2px',
    outline: 'transparent solid 2px',
    ':hover': {
      color: '#684BC7'
    },
    ':hover > span:last-child': {
      textDecoration: 'underline'
    },
    ':focus-visible': {
      outlineColor: '#684BC7'
    }
  },
  avatar: {
    display: 'inline-flex',
    alignItems: 'center' as 'center',
    justifyContent: 'center' as 'center',
    width: '20px',
    height: '20px',
    borderRadius: '50%',
    fontSize: '0.6875rem',
    fontWeight: 500,
    textAlign: 'center' as 'center',
    textTransform: 'uppercase' as 'uppercase',
    lineHeight: '20px'
  }
});
