import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

/** Displays a sortable table header cell with a sort indicator. */
export function TableHeaderCell(props: TableHeaderCell.Properties) {
  const onClick = () => {
    if(props.sortOrder === TableHeaderCell.SortOrder.ASCENDING) {
      props.onSort?.(TableHeaderCell.SortOrder.DESCENDING);
    } else {
      props.onSort?.(TableHeaderCell.SortOrder.ASCENDING);
    }
  };
  const isEnd = props.textAlign === 'end';
  const showIndicator =
    props.sortOrder !== TableHeaderCell.SortOrder.NONE;
  const [isHovered, setIsHovered] = React.useState(false);
  const indicatorVisible = showIndicator || isHovered;
  const indicator = (() => {
    if(!indicatorVisible) {
      return <div className={css(STYLES.indicatorSpacer)}/>;
    }
    const src = props.sortOrder === TableHeaderCell.SortOrder.DESCENDING ?
      'resources/account_page/profit_and_loss_page/sort-descending.svg' :
      'resources/account_page/profit_and_loss_page/sort-ascending.svg';
    const fillClass = showIndicator ?
      STYLES.sortIndicatorActive : STYLES.sortIndicatorInactive;
    return (
      <div className={css(STYLES.indicatorContainer)}>
        <img src={src} aria-hidden='true'
          className={css(STYLES.sortIndicator, fillClass)}/>
      </div>);
  })();
  return (
    <th className={css(STYLES.th)}
        aria-sort={ariaSort(props.sortOrder)}
        style={{textAlign: props.textAlign || 'start'}}>
      <button className={css(STYLES.button,
          indicatorVisible && STYLES.buttonWithIndicator,
          isEnd && STYLES.buttonEnd)}
          onClick={onClick}
          onMouseEnter={() => setIsHovered(true)}
          onMouseLeave={() => setIsHovered(false)}>
        {isEnd && <div className={css(STYLES.flexSpacer)}/>}
        <span>{props.children}</span>
        {indicator}
        {!isEnd && <div className={css(STYLES.flexSpacer)}/>}
      </button>
    </th>);
}

function ariaSort(
    order: TableHeaderCell.SortOrder): 'none' | 'ascending' | 'descending' {
  if(order === TableHeaderCell.SortOrder.ASCENDING) {
    return 'ascending';
  } else if(order === TableHeaderCell.SortOrder.DESCENDING) {
    return 'descending';
  }
  return 'none';
}

export namespace TableHeaderCell {

  /** The sort order of a column. */
  export enum SortOrder {

    /** The column is not sorted. */
    NONE,

    /** The column is sorted in increasing order. */
    ASCENDING,

    /** The column is sorted in decreasing order. */
    DESCENDING
  }

  export interface Properties {

    /** The sort order of the column. */
    sortOrder: SortOrder;

    /** The text alignment of the cell. */
    textAlign?: 'start' | 'end';

    /** Called when the user signals to change the sort order. */
    onSort?: (order: SortOrder) => void;

    /** The content of the header cell. */
    children?: React.ReactNode;
  }
}

const STYLES = StyleSheet.create({
  th: {
    backgroundColor: '#F8F8F8',
    color: '#5D5E6D',
    fontWeight: 500,
    padding: 0
  },
  button: {
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'flex-start',
    width: '100%',
    backgroundColor: 'transparent',
    borderWidth: 0,
    borderStyle: 'none',
    font: 'inherit',
    color: 'inherit',
    padding: '12px 19px',
    cursor: 'pointer',
    ':hover': {
      backgroundColor: '#E6E6E6'
    },
    ':focus-visible': {
      outline: '1px solid #684BC7',
      outlineOffset: '-1px'
    }
  },
  buttonWithIndicator: {
    paddingInlineEnd: '9px'
  },
  buttonEnd: {
    justifyContent: 'flex-end'
  },
  flexSpacer: {
    flex: 1
  },
  indicatorSpacer: {
    width: '14px',
    flex: '0 0 14px'
  },
  indicatorContainer: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '14px',
    flex: '0 0 14px',
    paddingLeft: '4px'
  },
  sortIndicator: {
    width: '10px',
    height: '10px'
  },
  sortIndicatorActive: {
    opacity: 1
  },
  sortIndicatorInactive: {
    opacity: 0.6
  }
});
