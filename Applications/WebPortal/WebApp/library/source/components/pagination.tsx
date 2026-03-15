import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { IconLabelButton } from './icon_label_button';

interface Properties {

  /** The current 0-based page index. */
  pageIndex: number;

  /** The maximum number of items per page. Defaults to 50. */
  pageSize?: number;

  /** The total number of items. */
  totalCount: number;

  /** Called when the user requests navigation to a page index. */
  onNavigate?: (page: number) => void;
}

interface State {
  isWide: boolean;
}

/** Displays pagination controls. */
export class Pagination extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.containerRef = React.createRef<HTMLDivElement>();
    this.state = {
      isWide: false
    };
  }

  public componentDidMount(): void {
    this.resizeObserver = new ResizeObserver(this.onResize);
    if(this.containerRef.current) {
      this.resizeObserver.observe(this.containerRef.current);
    }
  }

  public componentWillUnmount(): void {
    this.resizeObserver?.disconnect();
  }

  public render(): JSX.Element {
    const pageSize = this.props.pageSize ?? 50;
    const pageCount = Math.ceil(this.props.totalCount / pageSize);
    if(pageCount <= 1) {
      return (
        <div hidden ref={this.containerRef}/>);
    }
    const hasPrevious = this.props.pageIndex > 0;
    const hasNext = this.props.pageIndex < pageCount - 1;
    const {display, pageSequence} = this.computeDisplay(pageCount);
    const variant = this.state.isWide ?
      IconLabelButton.Variant.ICON_LABEL : undefined;
    return (
      <div ref={this.containerRef}>
        <div className={css(STYLES.content,
            this.state.isWide && STYLES.wideContent)}>
          <Previous variant={variant} disabled={!hasPrevious}
            onClick={() =>
              this.props.onNavigate?.(this.props.pageIndex - 1)}/>
          <div className={css(STYLES.pageLinks)}>
            {this.renderPageLinks(display, pageSequence, pageCount)}
          </div>
          <Next variant={variant} disabled={!hasNext}
            onClick={() =>
              this.props.onNavigate?.(this.props.pageIndex + 1)}/>
        </div>
      </div>);
  }

  private computeDisplay(pageCount: number):
      {display: Display, pageSequence: number[]} {
    const n = this.state.isWide ? 9 : 5;
    const offset = this.state.isWide ? 4 : 2;
    const padding = this.state.isWide ? 2 : 1;
    if(pageCount <= n) {
      return {
        display: Display.FULL,
        pageSequence: range(0, pageCount)
      };
    }
    const edgeSlots = this.state.isWide ? 2 : 1;
    const seqLen = n - edgeSlots;
    if(this.props.pageIndex <= offset) {
      return {
        display: Display.TRAILING_ELLIPSIS,
        pageSequence: range(0, seqLen)
      };
    }
    if(this.props.pageIndex >= (pageCount - 1) - offset) {
      return {
        display: Display.LEADING_ELLIPSIS,
        pageSequence: range(pageCount - seqLen, seqLen)
      };
    }
    const start = this.props.pageIndex - padding;
    const end = this.props.pageIndex + padding;
    return {
      display: Display.BOTH_ELLIPSES,
      pageSequence: range(start, end - start + 1)
    };
  }

  private renderPageLinks(display: Display,
      pageSequence: number[], pageCount: number): JSX.Element {
    const sequence = (
      <div className={css(STYLES.pageSequence)}>
        {pageSequence.map(i =>
          <PageLink key={i} index={i}
            isCurrent={i === this.props.pageIndex}
            onClick={() => this.props.onNavigate?.(i)}/>)}
      </div>);
    switch(display) {
      case Display.FULL:
        return sequence;
      case Display.TRAILING_ELLIPSIS:
        return (
          <>
            {sequence}
            <Ellipsis/>
            {this.state.isWide &&
              <PageLink index={pageCount - 1} isCurrent={false}
                onClick={() =>
                  this.props.onNavigate?.(pageCount - 1)}/>}
          </>);
      case Display.LEADING_ELLIPSIS:
        return (
          <>
            {this.state.isWide &&
              <PageLink index={0} isCurrent={false}
                onClick={() => this.props.onNavigate?.(0)}/>}
            <Ellipsis/>
            {sequence}
          </>);
      case Display.BOTH_ELLIPSES:
        return (
          <>
            {this.state.isWide &&
              <PageLink index={0} isCurrent={false}
                onClick={() => this.props.onNavigate?.(0)}/>}
            <Ellipsis/>
            {sequence}
            <Ellipsis/>
            {this.state.isWide &&
              <PageLink index={pageCount - 1} isCurrent={false}
                onClick={() =>
                  this.props.onNavigate?.(pageCount - 1)}/>}
          </>);
    }
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    const width = entries[0]?.contentRect.width ?? 0;
    const isWide = width >= 732;
    if(this.state.isWide !== isWide) {
      this.setState({isWide});
    }
  }

  private containerRef: React.RefObject<HTMLDivElement>;
  private resizeObserver?: ResizeObserver;
}

function range(start: number, count: number): number[] {
  const result: number[] = [];
  for(let i = 0; i < count; ++i) {
    result.push(start + i);
  }
  return result;
}

enum Display {
  FULL,
  TRAILING_ELLIPSIS,
  LEADING_ELLIPSIS,
  BOTH_ELLIPSES
}

function Previous(props: {variant?: IconLabelButton.Variant,
    disabled?: boolean, onClick?: () => void}) {
  return (
    <IconLabelButton
      icon='resources/arrow-previous.svg'
      label='Previous'
      aria-label='Previous Page'
      variant={props.variant}
      disabled={props.disabled}
      iconPlacement={IconLabelButton.Placement.LEADING}
      onClick={props.onClick}/>);
}

function Next(props: {variant?: IconLabelButton.Variant,
    disabled?: boolean, onClick?: () => void}) {
  return (
    <IconLabelButton
      icon='resources/arrow-next.svg'
      label='Next'
      aria-label='Next Page'
      variant={props.variant}
      disabled={props.disabled}
      iconPlacement={IconLabelButton.Placement.TRAILING}
      onClick={props.onClick}/>);
}

function Ellipsis() {
  return (
    <span className={css(STYLES.ellipsis)} role='presentation'>
      …
    </span>);
}

interface PageLinkProperties {
  index: number;
  isCurrent: boolean;
  onClick: () => void;
}

interface PageLinkState {
  isHovered: boolean;
  isFocused: boolean;
}

class PageLink extends React.Component<PageLinkProperties, PageLinkState> {
  constructor(props: PageLinkProperties) {
    super(props);
    this.state = {
      isHovered: false,
      isFocused: false
    };
  }

  public render(): JSX.Element {
    const baseBackground = (() => {
      if(this.props.isCurrent) {
        return '#684BC7';
      }
      if(this.state.isHovered || this.state.isFocused) {
        return '#F8F8F8';
      }
      return 'transparent';
    })();
    return (
      <a className={css(STYLES.pageLink)}
          href='#'
          aria-label={`Page ${this.props.index + 1}`}
          aria-current={this.props.isCurrent ? 'page' : 'false'}
          onClick={this.onClick}
          onMouseEnter={this.onMouseEnter}
          onMouseLeave={this.onMouseLeave}
          onFocus={this.onFocus}
          onBlur={this.onBlur}>
        <div className={css(STYLES.pageLinkBase)}
          style={{backgroundColor: baseBackground}}/>
        <span className={css(STYLES.pageLinkContent)}
          style={{color: this.props.isCurrent ? '#FFFFFF' : '#333333'}}>
          {this.props.index + 1}
        </span>
      </a>);
  }

  private onClick = (event: React.MouseEvent) => {
    event.preventDefault();
    this.props.onClick();
  }

  private onMouseEnter = () => {
    this.setState({isHovered: true});
  }

  private onMouseLeave = () => {
    this.setState({isHovered: false});
  }

  private onFocus = () => {
    this.setState({isFocused: true});
  }

  private onBlur = () => {
    this.setState({isFocused: false});
  }
}

const STYLES = StyleSheet.create({
  content: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    gap: '10px'
  },
  wideContent: {
    gap: '18px'
  },
  pageLinks: {
    display: 'flex',
    alignItems: 'center',
    flex: '0 1 auto',
    minWidth: 0,
    overflowX: 'auto'
  },
  pageSequence: {
    display: 'flex',
    alignItems: 'center',
    overflowX: 'auto'
  },
  ellipsis: {
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '34px',
    height: '34px',
    color: '#7D7E90'
  },
  pageLink: {
    position: 'relative',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    minWidth: '34px',
    height: '34px',
    border: '1px solid transparent',
    borderRadius: '1px',
    padding: '0 3px',
    textDecoration: 'none',
    boxSizing: 'border-box',
    outline: 'none',
    cursor: 'pointer',
    ':focus-visible': {
      borderColor: '#684BC7'
    }
  },
  pageLinkBase: {
    position: 'absolute',
    top: '2px',
    right: '2px',
    bottom: '2px',
    left: '2px',
    borderRadius: '2px'
  },
  pageLinkContent: {
    position: 'relative',
    fontSize: '14px',
    fontFamily: 'Roboto',
    textAlign: 'center'
  }
});
