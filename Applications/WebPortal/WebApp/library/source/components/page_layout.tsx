import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

/** Lays out its children centered horizontally with responsive widths. */
export class PageLayout extends React.Component<{}> {

  /** Scrolls the page content to the top. */
  public scrollToTop(): void {
    this.containerRef.current?.scrollTo(0, 0);
  }

  public render(): JSX.Element {
    return (
      <div ref={this.containerRef} className={css(PageLayout.STYLES.outer)}>
        <div className={css(PageLayout.STYLES.inner)}>
          {this.props.children}
        </div>
      </div>);
  }

  private static readonly STYLES = StyleSheet.create({
    outer: {
      flex: '1 1 auto',
      minHeight: 0,
      overflowY: 'auto',
      scrollbarGutter: 'stable',
      backgroundColor: '#FFFFFF'
    },
    inner: {
      width: 'min(100%, 460px)',
      marginLeft: 'auto',
      marginRight: 'auto',
      display: 'flex',
      flexDirection: 'column',
      minHeight: '100%',
      '@media (min-width: 768px) and (max-width: 1035px)': {
        width: 'min(100%, 768px)'
      },
      '@media (min-width: 1036px)': {
        width: 'min(100%, 1036px)'
      }
    }
  });
  private containerRef = React.createRef<HTMLDivElement>();
}
