import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel, Button, EditAccountGroupModal,
  EditListModal, LocalAccountGroupQueryModel } from 'web_portal';

interface Properties {
  selected: Beam.DirectoryEntry[];
  selectionMode: EditListModal.SelectionMode;
  readOnly: boolean;
  lookupDelay: number;
  failLookup: boolean;
  onSubmit: (selected: Beam.DirectoryEntry[]) => void;
  onClose: () => void;
}

interface State {
  isOpen: boolean;
}

/** Demonstrates asynchronous account and group lookup in the catalog. */
export class EditAccountGroupModalExample extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {isOpen: false};
    this.model = new ExampleModel(() => this.props.lookupDelay,
      () => this.props.failLookup);
  }

  public render(): JSX.Element {
    return (
      <div>
        <Button label='Open editor' onClick={this.onOpen}/>
        <p>Selection: {
          this.props.selected.map(entry => entry.name).join(', ') ||
            '(empty)'}</p>
        {this.state.isOpen &&
          <EditAccountGroupModal model={this.model}
            selected={this.props.selected}
            selectionMode={this.props.selectionMode}
            readOnly={this.props.readOnly}
            onSubmit={this.onSubmit} onClose={this.onClose}/>}
      </div>);
  }

  private onOpen = () => {
    this.setState({isOpen: true});
  };

  private onClose = () => {
    this.setState({isOpen: false});
    this.props.onClose();
  };

  private onSubmit = (selected: Beam.DirectoryEntry[]) => {
    this.props.onSubmit(selected);
    this.setState({isOpen: false});
  };

  private model: AccountGroupQueryModel;
}

class ExampleModel extends AccountGroupQueryModel {
  constructor(delay: () => number, fail: () => boolean) {
    super();
    this.delay = delay;
    this.fail = fail;
    this.model = new LocalAccountGroupQueryModel([
      Beam.DirectoryEntry.makeAccount(1, 'Alice'),
      Beam.DirectoryEntry.makeAccount(2, 'Alex'),
      Beam.DirectoryEntry.makeAccount(3, 'Bob'),
      Beam.DirectoryEntry.makeAccount(4, 'Charlie'),
      Beam.DirectoryEntry.makeDirectory(101, 'Alpha Group'),
      Beam.DirectoryEntry.makeDirectory(102, 'Beta Group'),
      Beam.DirectoryEntry.makeDirectory(103, 'Operations'),
      Beam.DirectoryEntry.makeAccount(5, 'Shared'),
      Beam.DirectoryEntry.makeDirectory(104, 'Shared'),
      Beam.DirectoryEntry.makeAccount(2, 'Alex')
    ]);
  }

  public async submit(prefix: string):
      Promise<readonly Beam.DirectoryEntry[]> {
    const fail = this.fail();
    await new Promise(resolve => window.setTimeout(resolve, this.delay()));
    if(fail) {
      throw new Error('The lookup service is unavailable.');
    }
    return this.model.submit(prefix);
  }

  private delay: () => number;
  private fail: () => boolean;
  private model: LocalAccountGroupQueryModel;
}
