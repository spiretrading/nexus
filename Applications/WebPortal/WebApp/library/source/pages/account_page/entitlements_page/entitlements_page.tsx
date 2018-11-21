import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import {DisplaySize, HBoxLayout, HLine, Padding, VBoxLayout} from '../../..';
import {SubmissionBox} from '../submission_box';
import {EntitlementRow} from './entitlement_row';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of markets. */
  marketDatabase: Nexus.MarketDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Indicates an entitlement has been clicked.
   * @param entitlement - The entitlement that was clicked.
   */
  onEntitlementClick?(entitlement: Beam.DirectoryEntry): void;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?(comment: string): void;
}

/* Displays a list of entitlements. */
export class EntitlementsPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    onEntitlementClick: () => {},
    onSubmit: () => {}
  };

  constructor(props: Properties) {
    super(props);
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
            onClick = {() => {
              this.props.onEntitlementClick(entry.group);
              this.forceUpdate();
              }
            }
            displaySize={this.props.displaySize}
            marketDatabase={this.props.marketDatabase}/>);
      }
      return <div>{rows}</div>;
    })();
    return (
      <HBoxLayout id='Page' width='100%' style={EntitlementsPage.STYLE.page}>
        <Padding/>
        <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
        <VBoxLayout>
          <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
          {entitlements}
          <Padding size={EntitlementsPage.LINE_PADDING}/>
          <HLine color={EntitlementsPage.LINE_COLOR}/>
          <Padding size={EntitlementsPage.LINE_PADDING}/>
          <SubmissionBox
            onSubmit={this.props.onSubmit}
            roles={this.props.roles}/>
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
