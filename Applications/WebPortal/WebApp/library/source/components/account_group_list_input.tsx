import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel, EditAccountGroupModal } from
  './edit_account_group_modal';
import { ListInput } from './list_input';

interface Properties extends Omit<React.InputHTMLAttributes<HTMLInputElement>,
    'value' | 'defaultValue' | 'onChange'> {

  /** The selected accounts and groups displayed in the input. */
  value: readonly Beam.DirectoryEntry[];

  /** The model used to look up accounts and groups. */
  model: AccountGroupQueryModel;

  /** Called when the user submits a new selection. */
  onChange?: (value: Beam.DirectoryEntry[]) => void;
}

/** Displays selected accounts and groups and opens their editor on focus. */
export class AccountGroupListInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <ListInput {...this.props}
      placeholder={this.props.placeholder ?? 'Enter accounts or groups'}
      title='Edit Accounts and Groups' listHeading='Added Accounts and Groups'
      getLabel={getLabel} editListModal={EditAccountGroupModal}/>;
  }
}

function getLabel(entry: Beam.DirectoryEntry): string {
  return entry.name;
}
