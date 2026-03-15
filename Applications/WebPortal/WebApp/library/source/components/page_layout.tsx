import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

/** Lays out its children centered horizontally with responsive widths. */
export class PageLayout extends React.Component<{}> {
  public render(): JSX.Element {
    return (
      <div className={css(PageLayout.STYLES.wrapper)}>
        {this.props.children}
      </div>);
  }

  private static readonly STYLES = StyleSheet.create({
    wrapper: {
      width: 'min(100%, 460px)',
      marginLeft: 'auto',
      marginRight: 'auto',
      backgroundColor: '#FFFFFF',
      flex: '1 1 auto',
      minHeight: 0,
      display: 'flex',
      flexDirection: 'column',
      '@media (min-width: 768px) and (max-width: 1035px)': {
        width: '768px'
      },
      '@media (min-width: 1036px)': {
        width: '1036px'
      }
    }
  });
}
