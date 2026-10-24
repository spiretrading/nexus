import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel } from
  '../../components/edit_account_group_modal';
import { EditListModal } from '../../components/edit_list_modal';
import { isReportRecipient, ReportRecipientQueryModel } from
  './report_recipient_query_model';

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
export class ShareReportModal extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.model = new ReportRecipientQueryModel(props.model);
  }

  public render(): JSX.Element {
    if(this.model.source !== this.props.model) {
      this.model = new ReportRecipientQueryModel(this.props.model);
    }
    return <EditListModal {...this.props} model={this.model}
      title='Share Report'
      listHeading='Added Recipients' placeholder='Enter account or group'
      submitLabel='Share' canSubmit={canSubmit} getLabel={getLabel}
      isEqual={isEqual}/>;
  }

  private model: ReportRecipientQueryModel;
}

function canSubmit(selected: readonly Beam.DirectoryEntry[]): boolean {
  return selected.length !== 0 && selected.every(isReportRecipient);
}

function getLabel(entry: Beam.DirectoryEntry): string {
  return entry.name;
}

function isEqual(first: Beam.DirectoryEntry, second: Beam.DirectoryEntry):
    boolean {
  return first.equals(second);
}
