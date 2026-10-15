import * as Nexus from 'nexus';
import * as React from 'react';
import { getScopeLabel, isScopeEqual, QueryModel } from '../models';
import { EditListModal } from './edit_list_modal';

interface Properties {

  /** The model used to look up scope entries and resolve CSV entries. */
  model: QueryModel<Nexus.Scope>;

  /** The committed countries, venues, tickers, or global scope entry. */
  selected: readonly Nexus.Scope[];

  /** Whether the selection can only be viewed. */
  readOnly?: boolean;

  /** Called with the selected scope entries when Submit is pressed. */
  onSubmit?: (selected: Nexus.Scope[]) => void;

  /** Called on dismissal. The caller should unmount the modal. */
  onClose?: () => void;
}

/** Edits the countries, venues, and tickers included in a scope. */
export class EditScopeModal extends React.Component<Properties> {
  public render(): JSX.Element {
    return <EditListModal {...this.props}
      title='Edit Scope' listHeading='Added Scope'
      placeholder='Enter country, venue, or ticker'
      getLabel={getScopeLabel} isEqual={isScopeEqual}/>;
  }
}
