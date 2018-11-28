import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HBoxLayout, HLine, Padding, VBoxLayout } from '../../..';
import { SubmissionBox } from '../submission_box';
import { EntitlementRow } from './entitlement_row';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** Whether the submit button is enabled. */
  isSubmitEnabled?: boolean;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of markets. */
  marketDatabase: Nexus.MarketDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Indicates an entitlement has been clicked. */
  onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?: (comment: string) => void;
}

/* Displays a list of entitlements. */
export class EntitlementsPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    isError: false,
    status: '',
    onEntitlementClick: () => {},
    onSubmit: () => {}
  }

  public render(): JSX.Element {
    const entitlements = (() => {
      const rows = [];
      for(const entry of this.props.entitlements) {
        rows.push(
          <EntitlementRow
            entitlementEntry={entry}
            currencyEntry=
              {this.props.currencyDatabase.fromCurrency(entry.currency)}
            isActive={this.props.checked.test(entry.group)}
            onClick = {() => this.props.onEntitlementClick(entry.group)}
            displaySize={this.props.displaySize}
            marketDatabase={this.props.marketDatabase}/>);
      }
      return <div>{rows}</div>;
    })();
    return (
      <HBoxLayout style={EntitlementsPage.STYLE.page}>
        <Padding/>
        <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
        <VBoxLayout>
          <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
          {entitlements}
          <Padding size={EntitlementsPage.LINE_PADDING}/>
          <HLine color={EntitlementsPage.LINE_COLOR}/>
          <Padding size={EntitlementsPage.LINE_PADDING}/>
          <SubmissionBox roles={this.props.roles} isError={this.props.isError}
            status={this.props.status} isEnabled={this.props.isSubmitEnabled}
            onSubmit={this.props.onSubmit}/>
          <Padding size={EntitlementsPage.BOTTOM_PADDING}/>
        </VBoxLayout>
        <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
        <Padding/>
      </HBoxLayout>);
  }

  private static readonly STYLE = {
    page: {
      overflow: 'auto',
      minWidth: '320px',
      width: '100%',
      height: '100%'
    },
    text: {
      font: '400 14px Roboto',
      textAlign: 'center' as 'center'
    }
  };
  private static readonly DEFAULT_PADDING = '18px';
  private static readonly BOTTOM_PADDING = '20px';
  private static readonly LINE_PADDING = '30px';
  private static readonly LINE_COLOR = '#E6E6E6';
}
