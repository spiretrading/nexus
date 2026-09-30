import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ScheduledReportItemContextMenu } from 'web_portal';

interface Properties extends Omit<
    React.ComponentProps<typeof ScheduledReportItemContextMenu>,
    'id' | 'invoker' | 'className'> {}

/** Demonstrates a scheduled report menu and its invoking button. */
export class ScheduledReportItemContextMenuExample extends
    React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.invoker = React.createRef();
  }

  public render(): JSX.Element {
    const id = 'catalog-scheduled-report-item-menu';
    return <>
      <button ref={this.invoker} type='button' {...{popovertarget: id}}
          className={css(STYLES.invoker)}>
        Open Menu
      </button>
      <ScheduledReportItemContextMenu {...this.props} id={id}
        invoker={this.invoker} className={css(STYLES.menu)}/>
    </>;
  }

  private invoker: React.RefObject<HTMLButtonElement>;
}

const STYLES = StyleSheet.create({
  invoker: {anchorName: '--catalog-scheduled-report-item-menu'},
  menu: {
    positionAnchor: '--catalog-scheduled-report-item-menu',
    positionArea: 'bottom span-right',
    positionTryFallbacks: 'flip-block flip-inline',
    margin: 0
  }
});
