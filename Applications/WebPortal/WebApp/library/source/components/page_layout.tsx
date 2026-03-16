import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

/** Lays out its children centered horizontally with responsive widths. */
export class PageLayout extends React.Component<{}> {
  public render(): JSX.Element {
    return (
      <div className={css(PageLayout.STYLES.outer)}>
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
        width: '768px'
      },
      '@media (min-width: 1036px)': {
        width: '1036px'
      }
    }
  });
}
