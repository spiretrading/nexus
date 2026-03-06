import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The category of the request. */
  category: Nexus.AccountModificationRequest.Type;
}

/** Displays a colored tag indicating the request's category. */
export function RequestCategoryTag(props: Properties) {
  const {label, color, backgroundColor} = getCategoryStyle(props.category);
  const dynamicStyle = StyleSheet.create({
    tag: {color, backgroundColor}
  });
  return (
    <span className={css(STYLES.tag, dynamicStyle.tag)}>
      {label}
    </span>);
}

function getCategoryStyle(category: Nexus.AccountModificationRequest.Type):
    {label: string, color: string, backgroundColor: string} {
  switch(category) {
    case Nexus.AccountModificationRequest.Type.RISK:
      return {
        label: 'Risk Controls',
        color: '#3B1886',
        backgroundColor: '#E2DFFF'
      };
    case Nexus.AccountModificationRequest.Type.ENTITLEMENTS:
      return {
        label: 'Entitlements',
        color: '#0B7063',
        backgroundColor: '#BFFFF5'
      };
    case Nexus.AccountModificationRequest.Type.COMPLIANCE:
      return {
        label: 'Compliance',
        color: '#70370B',
        backgroundColor: '#FFDDAF'
      };
  }
}

const STYLES = StyleSheet.create({
  tag: {
    display: 'inline-block',
    padding: '0 8px',
    borderRadius: '4px',
    fontSize: '0.75rem',
    fontFamily: 'Roboto',
    whiteSpace: 'nowrap' as 'nowrap'
  }
});
