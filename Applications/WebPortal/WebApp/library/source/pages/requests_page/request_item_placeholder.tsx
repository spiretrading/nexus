import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface State {
  isWide: boolean;
}

/** Displays a skeleton loading placeholder for a RequestItem. */
export class RequestItemPlaceholder extends React.Component<{}, State> {
  constructor(props: {}) {
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
        <div className={css(STYLES.header)}>
          <div className={css(STYLES.skeleton, STYLES.headerLeft)}/>
          <div className={css(STYLES.skeleton,
            this.state.isWide ?
              STYLES.headerRightWide : STYLES.headerRight)}/>
        </div>
        <div className={css(STYLES.metadata)}>
          <div className={css(STYLES.skeleton,
            this.state.isWide ?
              STYLES.metadataWide : STYLES.metadataNarrow)}/>
        </div>
        <div className={css(STYLES.changes)}>
          <div className={css(STYLES.skeleton, STYLES.changesSkeleton,
            !this.state.isWide && STYLES.changesSkeletonNarrow)}/>
        </div>
      </div>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 768;
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
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '11px 17px'
  },
  skeleton: {
    background: 'linear-gradient(to right, #EAEAEA, #FAFAFA)'
  },
  header: {
    display: 'flex',
    alignItems: 'center' as 'center',
    gap: '18px'
  },
  headerLeft: {
    width: '160px',
    height: '18px',
    flexShrink: 0
  },
  headerRight: {
    width: '60px',
    height: '18px',
    flexShrink: 0,
    marginLeft: 'auto'
  },
  headerRightWide: {
    width: '116px',
    height: '18px',
    flexShrink: 0,
    marginLeft: 'auto'
  },
  metadata: {
    marginTop: '4px'
  },
  metadataNarrow: {
    width: '60px',
    height: '16px'
  },
  metadataWide: {
    width: '284px',
    height: '16px'
  },
  changes: {
    marginTop: '10px'
  },
  changesSkeleton: {
    width: '284px',
    height: '24px'
  },
  changesSkeletonNarrow: {
    width: '100%'
  }
});
