import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { AccountDirectoryModel, FilterBar } from '.';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Called when the user wants to make a new group. */
  onNewGroupClick?: () => {};

  /** Called when the user wants to make a new account. */
  onNewAccountClick?: () => {};

  /** Model that contains imformation about the accounts. */
  model: AccountDirectoryModel;
}

interface State {
  filter: string;
}

/** Displays an directory of accounts. */
export class AccountDirectoryPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      filter: ''
    };
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    return (
      <div>
        <div id={'header'}>
          <FilterBar value={this.state.filter} onChange={this.onChange}/>
          <div>
            <button onClick={this.props.onNewAccountClick}>New Account</button>
            <button onClick={this.props.onNewGroupClick}>New Group</button>
          </div>
        </div>
        <div id={'group_cards'}>

        </div>
      </div>);
  }

  private onChange(newFilter: string) {
    this.setState({filter: newFilter});
  }
}
