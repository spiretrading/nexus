import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { RequestsModel } from './requests_model';

interface Properties {

  /** The status of the entitlement. */
  status: RequestsModel.EntitlementStatus;
}

/** Displays a colored tag indicating an entitlement's status. */
export function EntitlementsStatusTag(props: Properties) {
  const {label, color, backgroundColor} = getStatusStyle(props.status);
  const dynamicStyle = StyleSheet.create({ tag: {color, backgroundColor} });
  return (
    <span className={css(STYLES.tag, dynamicStyle.tag)}>
      {label}
    </span>);
}

function getStatusStyle(status: RequestsModel.EntitlementStatus):
    {label: string, color: string, backgroundColor: string} {
  switch(status) {
    case RequestsModel.EntitlementStatus.GRANTED:
      return {
        label: 'Granted',
        color: '#0C7032',
        backgroundColor: '#EBFFF2'
      };
    case RequestsModel.EntitlementStatus.REVOKED:
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
