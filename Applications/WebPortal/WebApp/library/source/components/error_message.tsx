import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Button } from './button';

interface Properties {

  /** The displayed message. */
  message: string;

  /** Called when the user requests to retry. */
  onRetry?: () => void;
}

interface State {
  isWide: boolean;
}

/** Displays an error message with a retry button. */
export class ErrorMessage extends React.Component<Properties, State> {
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
    return (
      <div ref={this.containerRef} className={css(STYLES.container)}>
        <div className={css(STYLES.content)}>
          <div className={css(STYLES.filler)}/>
          <img src='resources/components/error.svg'
            className={css(STYLES.errorIcon)}/>
          <div className={css(STYLES.spacer)}/>
          <span className={css(STYLES.message)}>{this.props.message}</span>
          {this.state.isWide && <>
            <div className={css(STYLES.spacer)}/>
            <Button label='Retry' onClick={this.props.onRetry}/>
          </>}
          <div className={css(STYLES.filler)}/>
        </div>
        {!this.state.isWide &&
          <div className={css(STYLES.actions)}>
            <Button label='Retry' onClick={this.props.onRetry}/>
          </div>}
      </div>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 732;
      if(isWide !== this.state.isWide) {
        this.setState({isWide});
      }
    }
  };

  private containerRef: React.RefObject<HTMLDivElement>;
  private resizeObserver?: ResizeObserver;
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: 'Roboto',
    backgroundColor: '#FFFFFF',
    display: 'flex',
    flexDirection: 'column',
    height: '100%'
  },
  content: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    flexGrow: 1
  },
  filler: {
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 0
  },
  errorIcon: {
    width: '44px',
    height: '44px',
    flexShrink: 0
  },
  spacer: {
    height: '18px',
    flexShrink: 0
  },
  message: {
    textAlign: 'center',
    color: '#333333',
    fontSize: '0.875rem'
  },
  actions: {
    position: 'fixed',
    bottom: 0,
    left: 0,
    right: 0,
    display: 'flex',
    justifyContent: 'center',
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    boxShadow: '0 0 6px rgb(0 0 0 / 25%)'
  }
});
