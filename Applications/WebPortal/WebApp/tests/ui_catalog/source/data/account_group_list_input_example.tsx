import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupListInput, AccountGroupQueryModel,
  LocalAccountGroupQueryModel } from 'web_portal';

interface Properties {
  value: Beam.DirectoryEntry[];
  readOnly: boolean;
  disabled: boolean;
  lookupDelay: number;
  failLookup: boolean;
  onChange: (value: Beam.DirectoryEntry[]) => void;
}

/** Demonstrates the account/group input and its asynchronous editor. */
export class AccountGroupListInputExample extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.model = new ExampleModel(
      () => this.props.lookupDelay, () => this.props.failLookup);
  }

  public render(): JSX.Element {
    return <AccountGroupListInput model={this.model} value={this.props.value}
      readOnly={this.props.readOnly} disabled={this.props.disabled}
      aria-label='Accounts and groups' onChange={this.props.onChange}/>;
  }

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

  public async submit(prefix: string): Promise<readonly Beam.DirectoryEntry[]> {
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
