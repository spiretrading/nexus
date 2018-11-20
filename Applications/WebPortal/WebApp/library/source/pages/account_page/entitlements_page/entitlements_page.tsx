import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HBoxLayout, Padding, VBoxLayout } from '../../..';
import { EntitlementRow } from './entitlement_row';
import { HLine } from '../../../components';
import { SubmitButton } from '../submit_button';
import { Center } from '../../../layouts';
import { SubmissionBox } from '../submission_box';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  //  New stuff
  /** The currency in which the entitlement is priced in. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The set of markets. */
  marketDatabase: Nexus.MarketDatabase;
  //  new stuff ends

  /** Indicates an entitlement has been clicked.
   * @param entitlement - The entitlement that was clicked.
   */ //which aspect? Checkmart? Dropdown arrow?
  onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?: (comment: string) => void;
}

//trader and manager look the same
//
export class EntitlementsPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    onEntitlementClick: () => {},
    onSubmit: () => {}
  };

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const data = (() => {
      const rows = [];
      for(const entry of this.props.entitlements) {
        console.log(entry.toJson());
        rows.push(
          <EntitlementRow
          entitlementEntry={entry}
          currencyEntry=
          {this.props.currencyDatabase.fromCurrency(entry.currency)}
          isActive={true}
          displaySize={this.props.displaySize}
          marketDatabase={this.props.marketDatabase}
          />);
        console.log('NEXT!');
      }
      return <div>{rows}</div>;
    })();
    return (
      <HBoxLayout id='Page' width='100%'>
       <Padding/>
        <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
        <VBoxLayout>
          <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
            {data}
            <Padding size={EntitlementsPage.LINE_PADDING}/>
            <HLine color={EntitlementsPage.LINE_COLOR}/>
            <Padding size={EntitlementsPage.LINE_PADDING}/>
            <SubmissionBox
            roles={this.props.roles}/>
          <Padding size={EntitlementsPage.BOTTOM_PADDING}/>
        </VBoxLayout>
       <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
      <Padding/>
      </HBoxLayout>
    );
  }

  private static readonly STYLE = {
    container: {
      border: '1px solid #C8C8C8',
      font: '400 14px Roboto',
      borderCollapse: 'collapse' as 'collapse',
      textAlign: 'center' as 'center',
      padding: 0,
      small: {
        minWidth: '284px',
        width: 'inherit' as 'inherit',
        maxWidth: '424px'
      },
      notSmall: {
        width: '636px'
      }
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

//footer ???
