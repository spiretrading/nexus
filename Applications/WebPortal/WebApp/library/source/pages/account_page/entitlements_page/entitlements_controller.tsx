import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, LoadingPage } from '../../..';
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

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

interface State {
  isLoaded: boolean;
  canSubmit: boolean;
  isError: boolean;
  status: string;
  comment: string;
  effectiveDate?: Beam.Date;
  selectedEntitlements: Beam.Set<Beam.DirectoryEntry>;
}

/** Implements a controller for the EntitlementsPage. */
export class EntitlementsController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      canSubmit: false,
      isError: false,
      status: '',
      comment: '',
      effectiveDate: Beam.Date.today(),
      selectedEntitlements: new Beam.Set<Beam.DirectoryEntry>()
    };
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    const hasDateError = this.state.effectiveDate == null;
    return <EntitlementsPage roles={this.props.roles}
      entitlements={this.props.entitlements}
      checked={this.state.selectedEntitlements}
      canSubmit={this.state.canSubmit && !hasDateError}
      comment={this.state.comment}
      effectiveDate={this.state.effectiveDate}
      dateError={hasDateError}
      isError={this.state.isError} status={this.state.status}
      currencyDatabase={this.props.currencyDatabase}
      venueDatabase={this.props.venueDatabase}
      displaySize={this.props.displaySize}
      onComment={this.onComment}
      onEffectiveDate={this.onEffectiveDate}
      onEntitlementClick={this.onEntitlementClick}
      onSubmit={this.onSubmit}/>;
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true,
          selectedEntitlements: this.props.model.entitlements
        });
      });
  }

  private onComment = (comment: string) => {
    this.setState({comment});
  }

  private onEffectiveDate = (effectiveDate?: Beam.Date) => {
    this.setState({effectiveDate});
  }

  private onEntitlementClick = (entitlement: Beam.DirectoryEntry) => {
    const next = this.state.selectedEntitlements.clone();
    if(next.test(entitlement)) {
      next.remove(entitlement);
    } else {
      next.add(entitlement);
    }
    this.setState({
      canSubmit: true,
      status: '',
      selectedEntitlements: next
    });
  }

  private onSubmit = async () => {
    try {
      this.setState({
        canSubmit: false,
        isError: false,
        status: ''
      });
      await this.props.model.submit(this.state.comment,
        this.state.selectedEntitlements,
        this.state.effectiveDate ?? Beam.Date.today());
      this.setState({
        status: 'Saved'
      });
    } catch(e) {
      this.setState({
        canSubmit: true,
        isError: true,
        status: 'Server issue'
      });
    }
  }
}
