import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

export enum Category {
  RISK_CONTROLS,
  ENTITLEMENTS,
  COMPLIANCE
}

interface Properties {

  /** The category of the request. */
  category: Category;
}

/** Displays a colored tag indicating the request's category. */
export class RequestCategoryTag extends React.Component<Properties> {
  public render(): JSX.Element {
    const {label, color, backgroundColor} =
      getCategoryStyle(this.props.category);
    const dynamicStyle = StyleSheet.create({
      tag: {color, backgroundColor}
    });
    return (
      <span className={css(STYLES.tag, dynamicStyle.tag)}>
        {label}
      </span>);
  }
}

function getCategoryStyle(category: Category):
    {label: string, color: string, backgroundColor: string} {
  switch(category) {
    case Category.RISK_CONTROLS:
      return {
        label: 'Risk Controls',
        color: '#3B1886',
        backgroundColor: '#E2DFFF'
      };
    case Category.ENTITLEMENTS:
      return {
        label: 'Entitlements',
        color: '#0B7063',
        backgroundColor: '#BFFFF5'
      };
    case Category.COMPLIANCE:
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
