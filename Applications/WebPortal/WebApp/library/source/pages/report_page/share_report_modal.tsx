import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel } from
  '../../components/edit_account_group_modal';
import { EditListModal } from '../../components/edit_list_modal';

interface Properties {

  /** The model used to look up recipient accounts and groups. */
  model: AccountGroupQueryModel;

  /** The initial recipients. Changes reset the working selection. */
  selected: readonly Beam.DirectoryEntry[];

  /** Called with the recipients when Share is pressed. */
  onSubmit?: (selected: Beam.DirectoryEntry[]) => void;

  /** Called on dismissal. The caller should unmount the modal. */
  onClose?: () => void;
}

/** Selects accounts and groups with which to share a report. */
export function ShareReportModal(props: Properties): JSX.Element {
  return <EditListModal {...props} title='Share Report'
    listHeading='Added Recipients' placeholder='Enter account or group'
    submitLabel='Share' canSubmit={canSubmit} getLabel={getLabel}
    isEqual={isEqual}/>;
}

function canSubmit(selected: readonly Beam.DirectoryEntry[]): boolean {
  return selected.length !== 0;
}

function getLabel(entry: Beam.DirectoryEntry): string {
  return entry.name;
}

function isEqual(first: Beam.DirectoryEntry, second: Beam.DirectoryEntry):
    boolean {
  return first.equals(second);
}
