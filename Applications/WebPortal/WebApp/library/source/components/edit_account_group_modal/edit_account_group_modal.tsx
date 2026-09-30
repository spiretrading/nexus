import * as Beam from 'beam';
import * as React from 'react';
import { EditListModal } from '../edit_list_modal';
import { AccountGroupQueryModel } from './account_group_query_model';

interface Properties {

  /** The model used to look up accounts and groups. */
  model: AccountGroupQueryModel;

  /** The committed selection of accounts and groups. */
  selected: readonly Beam.DirectoryEntry[];

  /** Whether the selection can only be viewed. */
  readOnly?: boolean;

  /** Called with the selected entries when Submit is pressed. */
  onSubmit?: (selected: Beam.DirectoryEntry[]) => void;

  /** Called on dismissal. The caller should unmount the modal. */
  onClose?: () => void;
}

/** Edits a selection of accounts and groups using asynchronous name lookup. */
export class EditAccountGroupModal extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <EditListModal title='Edit Accounts and Groups'
        listHeading='Added Accounts and Groups'
        placeholder='Enter account or group'
        selected={this.props.selected}
        readOnly={this.props.readOnly}
        model={this.props.model} getLabel={getLabel}
        isEqual={isEqual}
        onSubmit={this.props.onSubmit} onClose={this.props.onClose}/>);
  }
}

function getLabel(entry: Beam.DirectoryEntry): string {
  return entry.name;
}

function isEqual(first: Beam.DirectoryEntry, second: Beam.DirectoryEntry):
    boolean {
  return first.equals(second);
}
