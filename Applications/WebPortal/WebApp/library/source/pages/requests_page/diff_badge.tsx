import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { RequestsModel } from './requests_model';

interface Properties {

  /** The change value. */
  value: string;

  /** The direction of the change. */
  direction: RequestsModel.Direction;
}

/** Displays a badge indicating a change value and its direction. */
export function DiffBadge(props: Properties) {
  const {color, backgroundColor, sign} = getDirectionStyle(props.direction);
  const dynamicStyle = StyleSheet.create({ tag: {color, backgroundColor} });
  const hasSign = props.direction !== RequestsModel.Direction.NONE;
  const displayValue = hasSign && props.value.startsWith('-') ?
    props.value.substring(1) : props.value;
  return (
    <span className=
        {css(hasSign ? STYLES.tagWithSign : STYLES.tag, dynamicStyle.tag)}>
      {hasSign && <span className={css(STYLES.sign)}>{sign}</span>}
      {displayValue}
    </span>);
}

function getDirectionStyle(direction: RequestsModel.Direction):
    {color: string, backgroundColor: string, sign: string} {
  switch(direction) {
    case RequestsModel.Direction.POSITIVE:
      return {
        color: '#29C764',
        backgroundColor: '#EBFFF2',
        sign: '+'
      };
    case RequestsModel.Direction.NEGATIVE:
      return {
        color: '#E63F44',
        backgroundColor: '#FFF1F1',
        sign: '-'
      };
    case RequestsModel.Direction.NONE:
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
