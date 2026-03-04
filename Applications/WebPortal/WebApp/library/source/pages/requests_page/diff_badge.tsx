import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The change value. */
  value: string;

  /** The direction of the change. */
  direction: DiffBadge.Direction;
}

/** Displays a badge indicating a change value and its direction. */
export class DiffBadge extends React.Component<Properties> {
  public render(): JSX.Element {
    const {color, backgroundColor, sign} =
      getDirectionStyle(this.props.direction);
    const dynamicStyle = StyleSheet.create({
      tag: {color, backgroundColor}
    });
    const hasSign = this.props.direction !== DiffBadge.Direction.NONE;
    return (
      <span className={css(
          hasSign ? STYLES.tagWithSign : STYLES.tag, dynamicStyle.tag)}>
        {hasSign &&
          <span className={css(STYLES.sign)}>{sign}</span>}
        {this.props.value}
      </span>);
  }
}

export namespace DiffBadge {
  export enum Direction {
    POSITIVE,
    NEGATIVE,
    NONE
  }
}

function getDirectionStyle(direction: DiffBadge.Direction):
    {color: string, backgroundColor: string, sign: string} {
  switch(direction) {
    case DiffBadge.Direction.POSITIVE:
      return {
        color: '#29C764',
        backgroundColor: '#EBFFF2',
        sign: '+'
      };
    case DiffBadge.Direction.NEGATIVE:
      return {
        color: '#E63F44',
        backgroundColor: '#FFF1F1',
        sign: '-'
      };
    case DiffBadge.Direction.NONE:
      return {
        color: '#333333',
        backgroundColor: '#F8F8F8',
        sign: ''
      };
  }
}

const STYLES = StyleSheet.create({
  tag: {
    display: 'inline-block',
    padding: '4px 8px',
    borderRadius: '4px',
    fontSize: '0.875rem',
    fontFamily: 'Roboto'
  },
  tagWithSign: {
    display: 'inline-block',
    padding: '4px 8px 4px 4px',
    borderRadius: '4px',
    fontSize: '0.875rem',
    fontFamily: 'Roboto'
  },
  sign: {
    display: 'inline-block',
    width: '12px',
    marginRight: '4px',
    textAlign: 'center'
  }
});
