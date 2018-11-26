import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { EntitlementsModel } from './entitlements_model';
import { EntitlementsPage } from './entitlements_page';

interface Properties {

  /** The roles belonging to the account that's logged in. */
  roles: Nexus.AccountRoles;

  /** The model representing the account's entitlements. */
  model: EntitlementsModel;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of markets. */
  marketDatabase: Nexus.MarketDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

interface State {
  isLoaded: boolean;
  disabled: boolean;
  selectedEntitlements: Beam.Set<Beam.DirectoryEntry>;
}

/** Implements a controller for the EntitlementsPage. */
export class EntitlementsController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      disabled: true,
      selectedEntitlements: new Beam.Set<Beam.DirectoryEntry>()
    };
    this.onEntitlementClick = this.onEntitlementClick.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <div/>;
    }
    return <EntitlementsPage roles={this.props.roles}
      entitlements={this.props.entitlements}
      checked={this.state.selectedEntitlements}
      disabled={this.state.disabled}
      currencyDatabase={this.props.currencyDatabase}
      marketDatabase={this.props.marketDatabase}
      displaySize={this.props.displaySize}
      onEntitlementClick={this.onEntitlementClick} onSubmit={this.onSubmit}/>;
  }

  public componentWillMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true,
          selectedEntitlements: this.props.model.entitlements
        });
      });
  }

  private onEntitlementClick(entitlement: Beam.DirectoryEntry) {
    if(this.state.selectedEntitlements.test(entitlement)) {
      this.state.selectedEntitlements.remove(entitlement);
    } else {
      this.state.selectedEntitlements.add(entitlement);
    }
    this.setState({
      disabled: false,
      selectedEntitlements: this.state.selectedEntitlements
    });
  }

  private async onSubmit(comment: string) {
    try {
      this.setState({
        disabled: true
      });
      await this.props.model.submit(comment, this.state.selectedEntitlements);
      this.setState({
        disabled: false
      });
    } catch(e) {
    }
  }
}
