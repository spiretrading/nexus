import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { PageLayout } from '../../..';

interface Properties {
}

/** Displays the group profit and loss page. */
export class GroupProfitAndLossPage extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <PageLayout>
        <main className={css(STYLES.main)}>
        </main>
      </PageLayout>);
  }
}

const STYLES = StyleSheet.create({
  main: {
    padding: '18px 18px 166px',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontWeight: 400,
    color: '#333333'
  }
});
