import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.ThHTMLAttributes<HTMLTableCellElement>, 'aria-sort'> {

  /** The column's sort order. Defaults to NONE. */
  sortOrder?: SortableTableHeaderCell.SortOrder;

  /** The alignment of the contents. Defaults to start. */
  textAlign?: 'start' | 'end';

  /** Whether sorting is unavailable. */
  disabled?: boolean;

  /** Called with the requested sort order when the button is activated. */
  onSort?: (order: SortableTableHeaderCell.SortOrder) => void;
}

interface State {
  isHovered: boolean;
}

/** A table header that requests ascending or descending column sorting. */
export class SortableTableHeaderCell extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {isHovered: false};
  }

  public render(): JSX.Element {
    const {sortOrder = SortableTableHeaderCell.SortOrder.NONE,
      textAlign = 'start', disabled, onSort, children, className, style,
      ...attributes} = this.props;
    const showIndicator = !disabled && (this.state.isHovered ||
      sortOrder !== SortableTableHeaderCell.SortOrder.NONE);
    return <th {...attributes} scope={this.props.scope ?? 'col'}
        aria-sort={getAriaSort(sortOrder)}
        className={[css(STYLES.cell), className].join(' ')}
        style={{textAlign, ...style}}>
      <SortButton sortOrder={sortOrder} showIndicator={showIndicator}
          isEnd={(style?.textAlign ?? textAlign) === 'end'}
          disabled={disabled} onClick={this.onClick}
          onMouseEnter={this.onMouseEnter} onMouseLeave={this.onMouseLeave}>
        {children}
      </SortButton>
    </th>;
  }

  private onClick = () => {
    if(this.props.disabled) {
      return;
    }
    if(this.props.sortOrder === SortableTableHeaderCell.SortOrder.ASCENDING) {
      this.props.onSort?.(SortableTableHeaderCell.SortOrder.DESCENDING);
    } else {
      this.props.onSort?.(SortableTableHeaderCell.SortOrder.ASCENDING);
    }
  }

  private onMouseEnter = () => {
    this.setState({isHovered: true});
  }

  private onMouseLeave = () => {
    this.setState({isHovered: false});
  }
}

export namespace SortableTableHeaderCell {

  /** The sort order of a column. */
  export enum SortOrder {

    /** The column is not sorted. */
    NONE,

    /** The column is sorted in increasing order. */
    ASCENDING,

    /** The column is sorted in decreasing order. */
    DESCENDING
  }
}

interface SortButtonProperties extends
    React.ButtonHTMLAttributes<HTMLButtonElement> {
  sortOrder: SortableTableHeaderCell.SortOrder;
  showIndicator: boolean;
  isEnd: boolean;
}

function SortButton(props: SortButtonProperties): JSX.Element {
  const {sortOrder, showIndicator, isEnd, children, ...attributes} = props;
  return <button {...attributes} type='button'
      className={css(STYLES.button, isEnd && STYLES.buttonEnd)}>
    <span>{children}</span>
    <Indicator sortOrder={sortOrder} visible={showIndicator}/>
  </button>;
}

interface IndicatorProperties {
  sortOrder: SortableTableHeaderCell.SortOrder;
  visible: boolean;
}

function Indicator(props: IndicatorProperties): JSX.Element {
  return <div className={css(STYLES.indicator,
      !props.visible && STYLES.indicatorHidden)}>
    <svg width='10' height='10' viewBox='0 0 10 10'
        aria-hidden='true' focusable='false'
        className={css(STYLES.icon,
          props.sortOrder !== SortableTableHeaderCell.SortOrder.NONE &&
            STYLES.iconSorted)}>
      <path d={(() => {
        if(props.sortOrder === SortableTableHeaderCell.SortOrder.DESCENDING) {
          return 'M5.44721 8.60557C5.26295 8.9741 4.73705 8.9741 ' +
            '4.55279 8.60557L1.3618 2.22361C1.19558 1.89116 1.43733 1.5 ' +
            '1.80902 1.5L8.19098 1.5C8.56267 1.5 8.80442 1.89116 ' +
            '8.6382 2.22361L5.44721 8.60557Z';
        }
        return 'M4.55279 1.89443C4.73705 1.5259 5.26295 1.5259 ' +
          '5.44721 1.89443L8.6382 8.27639C8.80442 8.60884 8.56267 9 ' +
          '8.19098 9H1.80902C1.43733 9 1.19558 8.60884 1.3618 ' +
          '8.27639L4.55279 1.89443Z';
      })()}/>
    </svg>
  </div>;
}

function getAriaSort(order: SortableTableHeaderCell.SortOrder):
    'none' | 'ascending' | 'descending' {
  if(order === SortableTableHeaderCell.SortOrder.ASCENDING) {
    return 'ascending';
  } else if(order === SortableTableHeaderCell.SortOrder.DESCENDING) {
    return 'descending';
  }
  return 'none';
}

const STYLES = StyleSheet.create({
  cell: {
    boxSizing: 'border-box',
    backgroundColor: '#F8F8F8',
    color: '#5D5E6D',
    fontWeight: 500,
    padding: 0
  },
  button: {
    boxSizing: 'border-box',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'flex-start',
    width: '100%',
    backgroundColor: 'transparent',
    border: 'none',
    font: 'inherit',
    color: 'inherit',
    textAlign: 'inherit',
    padding: '12px 20px',
    paddingInlineEnd: '9px',
    cursor: 'pointer',
    ':hover': {backgroundColor: '#E6E6E6'},
    ':focus-visible': {
      outline: '1px solid #684BC7',
      outlineOffset: '-1px'
    },
    ':disabled': {cursor: 'default'}
  },
  buttonEnd: {justifyContent: 'flex-end'},
  indicator: {
    display: 'flex',
    alignItems: 'center',
    alignSelf: 'stretch',
    flex: '0 0 auto'
  },
  indicatorHidden: {visibility: 'hidden'},
  icon: {
    boxSizing: 'content-box',
    paddingInlineStart: '4px',
    flex: '0 0 auto',
    fill: '#A4A5B1'
  },
  iconSorted: {fill: '#7D7E90'}
});
