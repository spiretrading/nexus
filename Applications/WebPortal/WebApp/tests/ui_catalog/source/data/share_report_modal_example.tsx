import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel, Button, LocalAccountGroupQueryModel,
  ShareReportModal } from 'web_portal';

interface Properties {

  /** The recipients selected when the modal opens. */
  selected: Beam.DirectoryEntry[];

  /** The simulated account and group lookup delay, in milliseconds. */
  lookupDelay: number;

  /** Reports the submitted recipients. */
  onSubmit?: (selected: Beam.DirectoryEntry[]) => void;

  /** Reports dismissal without sharing. */
  onClose?: () => void;
}

interface State {
  open: boolean;
}

/** Demonstrates sharing with accounts and groups using a local model. */
export class ShareReportModalExample extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {open: false};
    this.model = new ExampleModel(() => this.props.lookupDelay);
  }

  public render(): JSX.Element {
    return <>
      <Button label='Open Share Report' onClick={this.onOpen}/>
      {this.state.open && <ShareReportModal model={this.model}
        selected={this.props.selected} onSubmit={this.onSubmit}
        onClose={this.onClose}/>}
    </>;
  }

  private onOpen = () => {
    this.setState({open: true});
  };

  private onSubmit = (selected: Beam.DirectoryEntry[]) => {
    this.setState({open: false});
    this.props.onSubmit?.(selected);
  };

  private onClose = () => {
    this.setState({open: false});
    this.props.onClose?.();
  };

  private model: AccountGroupQueryModel;
}

class ExampleModel extends LocalAccountGroupQueryModel {
  constructor(delay: () => number) {
    super([
      Beam.DirectoryEntry.makeAccount(1, 'Alice'),
      Beam.DirectoryEntry.makeAccount(2, 'Alex'),
      Beam.DirectoryEntry.makeAccount(3, 'Bob'),
      Beam.DirectoryEntry.makeDirectory(101, 'Alpha Group'),
      Beam.DirectoryEntry.makeDirectory(102, 'Beta Group')]);
    this.delay = delay;
  }

  public async submit(query: string): Promise<readonly Beam.DirectoryEntry[]> {
    await new Promise(resolve => window.setTimeout(resolve, this.delay()));
    return super.submit(query);
  }

  private delay: () => number;
}
