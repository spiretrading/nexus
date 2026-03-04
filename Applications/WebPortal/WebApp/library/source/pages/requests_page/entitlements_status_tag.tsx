import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The status of the entitlement. */
  status: EntitlementsStatusTag.Status;
}

/** Displays a colored tag indicating an entitlement's status. */
export class EntitlementsStatusTag extends React.Component<Properties> {
  public render(): JSX.Element {
    const {label, color, backgroundColor} =
      getStatusStyle(this.props.status);
    const dynamicStyle = StyleSheet.create({
      tag: {color, backgroundColor}
    });
    return (
      <span className={css(STYLES.tag, dynamicStyle.tag)}>
        {label}
      </span>);
  }
}

export namespace EntitlementsStatusTag {
  export enum Status {
    GRANTED,
    REVOKED
  }
}

function getStatusStyle(status: EntitlementsStatusTag.Status):
    {label: string, color: string, backgroundColor: string} {
  switch(status) {
    case EntitlementsStatusTag.Status.GRANTED:
      return {
        label: 'Granted',
        color: '#0C7032',
        backgroundColor: '#EBFFF2'
      };
    case EntitlementsStatusTag.Status.REVOKED:
      return {
        label: 'Revoked',
        color: '#7D7E90',
        backgroundColor: '#F8F8F8'
      };
  }
}

const STYLES = StyleSheet.create({
  tag: {
    display: 'inline-block',
    padding: '4px',
    borderRadius: '4px',
    fontSize: '0.75rem',
    fontWeight: 500,
    fontFamily: 'Roboto',
    textAlign: 'center',
    minWidth: '60px'
  }
});
