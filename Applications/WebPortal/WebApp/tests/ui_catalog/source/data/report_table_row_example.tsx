import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ReportTableRow } from 'web_portal';

interface Properties extends React.ComponentProps<typeof ReportTableRow> {

  /** Called instead of navigating away from the catalog. */
  onNavigate?: (url: string) => void;
}

/** Demonstrates a generated report row in its containing table. */
export class ReportTableRowExample extends React.Component<Properties> {
  public render(): JSX.Element {
    const {onNavigate, ...properties} = this.props;
    return <div className={css(STYLES.container)}>
      <table className={css(STYLES.table)}>
        <colgroup className={css(STYLES.columns)}>
          <col style={{width: '48px'}}/>
          <col style={{width: 'min(25cqw, 180px)'}}/>
          <col/>
          <col style={{width: 'min(25cqw, 132px)'}}/>
        </colgroup>
        <tbody className={css(STYLES.body)}>
          <ReportTableRow {...properties} onClick={this.onClick}/>
        </tbody>
      </table>
    </div>;
  }

  private onClick = (event: React.MouseEvent<HTMLTableRowElement>) => {
    const link = (event.target as HTMLElement).closest('a');
    if(link) {
      event.preventDefault();
      this.props.onNavigate?.(link.getAttribute('href'));
    }
  };
}

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%'},
  table: {
    width: '100%',
    maxWidth: '696px',
    borderSpacing: 0,
    tableLayout: 'fixed',
    '@container (width <= 424px)': {display: 'block'}
  },
  columns: {
    '@container (width <= 424px)': {display: 'none'}
  },
  body: {
    '@container (width <= 424px)': {display: 'block'}
  }
});
