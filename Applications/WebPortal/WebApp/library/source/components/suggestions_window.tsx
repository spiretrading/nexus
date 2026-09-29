import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties<T> {

  /** The input that anchors the window. */
  anchor: HTMLElement;

  /** The listbox identifier. */
  id: string;

  /** The available suggestions, in model order. */
  items: readonly T[];

  /** The highlighted suggestion's index. */
  selected: number;

  /** Returns the text displayed for a suggestion. */
  getLabel: (item: T) => string;

  /** Whether suggestions are being loaded. */
  loading: boolean;

  /** An error to display in the window. */
  error: string;

  /** Called when the pointer highlights a suggestion. */
  onHighlight: (index: number) => void;

  /** Called when a suggestion is chosen. */
  onSubmit: (item: T) => void;
}

/** Displays a scrollable suggestion list above surrounding page content. */
export class SuggestionsWindow<T> extends React.Component<Properties<T>> {
  constructor(props: Properties<T>) {
    super(props);
    this.window = React.createRef<HTMLDivElement>();
    this.observer = null;
  }

  public componentDidMount(): void {
    this.updatePosition();
    this.window.current.showPopover();
    this.observer = new ResizeObserver(this.updatePosition);
    this.observer.observe(this.props.anchor);
    window.addEventListener('resize', this.updatePosition);
    window.addEventListener('scroll', this.onScroll, true);
  }

  public componentDidUpdate(): void {
    this.window.current.querySelector(
      '[aria-selected="true"]')?.scrollIntoView({block: 'nearest'});
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    window.removeEventListener('resize', this.updatePosition);
    window.removeEventListener('scroll', this.onScroll, true);
    this.window.current.hidePopover();
  }

  public render(): JSX.Element {
    return <div ref={this.window} {...{popover: 'manual'}}
        className={css(STYLES.window)} onMouseDown={this.onMouseDown}>
      <ul id={this.props.id} role='listbox' aria-label='Suggestions'
          tabIndex={-1} className={css(STYLES.list)}>
        {this.props.items.map((item, index) =>
          <li key={index} id={`${this.props.id}-${index}`} role='option'
              aria-selected={index === this.props.selected}
              className={css(STYLES.item)}
              onPointerEnter={() => this.props.onHighlight(index)}
              onClick={() => this.props.onSubmit(item)}>
            {this.props.getLabel(item)}
          </li>)}
        {this.props.error &&
          <li role='presentation' className={css(STYLES.message)}>
            <span role='alert'>{this.props.error}</span>
          </li>}
        {!this.props.loading && !this.props.error &&
            this.props.items.length === 0 &&
          <li role='presentation' className={css(STYLES.message)}>
            No matches
          </li>}
      </ul>
    </div>;
  }

  private updatePosition = () => {
    const bounds = this.props.anchor.getBoundingClientRect();
    Object.assign(this.window.current.style, {
      left: `${bounds.left}px`, top: `${bounds.bottom + 4}px`,
      width: `${bounds.width}px`
    });
  };

  private onScroll = (event: Event) => {
    if(!this.window.current.contains(event.target as Node)) {
      this.updatePosition();
    }
  };

  private onMouseDown = (event: React.MouseEvent) => {
    event.preventDefault();
  };

  private window: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
}

const STYLES = StyleSheet.create({
  window: {
    position: 'fixed',
    inset: 'auto',
    boxSizing: 'border-box',
    height: '136px',
    margin: 0,
    padding: 0,
    border: '1px solid #C8C8C8',
    background: '#FFFFFF',
    color: '#333333',
    boxShadow: '0 2px 6px rgb(0 0 0 / 40%)',
    font: '400 14px Roboto',
    transition: 'opacity 200ms ease-in',
    '@starting-style': {opacity: 0}
  },
  list: {
    height: '100%',
    overflowY: 'auto',
    listStyle: 'none',
    margin: 0,
    padding: 0
  },
  item: {
    boxSizing: 'border-box',
    padding: '9px 10px',
    lineHeight: '16px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
    cursor: 'pointer',
    ':is([aria-selected="true"])': {
      background: '#684BC7', color: '#FFFFFF'
    }
  },
  message: {padding: '9px 10px'}
});
