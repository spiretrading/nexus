import * as React from 'react';
import { ContextMenu } from '../../../components';

interface Properties extends
    Omit<React.ComponentProps<typeof ContextMenu>, 'items' | 'onSubmit'> {

  /** Called when a scheduled report command is chosen. */
  onSubmit?: (command: ScheduledReportItemContextMenu.Command) => void;
}

/** Provides run, duplicate, and delete commands for a scheduled report. */
export function ScheduledReportItemContextMenu(props: Properties): JSX.Element {
  return <ContextMenu {...props} items={ITEMS}/>;
}

export namespace ScheduledReportItemContextMenu {

  /** A command that can be applied to a scheduled report. */
  export type Command = 'run' | 'duplicate' | 'delete';
}

const ITEMS: ContextMenu.Entry[] = [
  {body: <ContextMenu.Item label='Run Now'/>, value: 'run'},
  {body: <ContextMenu.Item label='Duplicate'/>, value: 'duplicate'},
  {body: <ContextMenu.Item label='Delete'/>, value: 'delete'}
];
