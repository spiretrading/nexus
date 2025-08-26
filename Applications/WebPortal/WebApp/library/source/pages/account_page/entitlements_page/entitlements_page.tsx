import * as Beam from 'beam';
import { Padding, VBoxLayout } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine } from '../../..';
import { SubmissionInput } from '..';
import { EntitlementRow } from './entitlement_row';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** Whether the submit button is enabled. */
  canSubmit?: boolean;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Indicates an entitlement has been clicked. */
  onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?: (comment: string) => void;
}

interface State {
  comment: string;
}

/* Displays a list of entitlements. */
export class EntitlementsPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    canSubmit: false,
    isError: false,
    status: '',
    onEntitlementClick: () => {},
    onSubmit: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      comment: ''
    }
    this.onCommentChange = this.onCommentChange.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
  }

  public render(): JSX.Element {
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return EntitlementsPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return EntitlementsPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return EntitlementsPage.STYLE.contentLarge;
      }
    })();
    const entitlements = (() => {
      const rows = [];
      for(const entry of this.props.entitlements) {
        rows.push(
          <EntitlementRow
            key={entry.group.id}
            entitlementEntry={entry}
            currencyEntry=
              {this.props.currencyDatabase.fromCurrency(entry.currency)}
            isActive={this.props.checked.test(entry.group)}
            onClick={() => this.props.onEntitlementClick(entry.group)}
            displaySize={this.props.displaySize}
            venueDatabase={this.props.venueDatabase}/>);
      }
      return <div>{rows}</div>;
    })();
    return (
      <div style={EntitlementsPage.STYLE.page}>
        <div style={EntitlementsPage.STYLE.pagePadding}/>
        <div style={contentWidth}>
          <VBoxLayout width='100%'>
            <Padding size={EntitlementsPage.DEFAULT_PADDING}/>
            {entitlements}
            <Padding size={EntitlementsPage.LINE_PADDING}/>
            <HLine color={EntitlementsPage.LINE_COLOR}/>
            <Padding size={EntitlementsPage.LINE_PADDING}/>
            <SubmissionInput comment={this.state.comment}
              roles={this.props.roles} isError={this.props.isError}
              status={this.props.status} isEnabled={this.props.canSubmit}
              onChange={this.onCommentChange}
              onSubmit={this.onSubmit}/>
            <Padding size={EntitlementsPage.BOTTOM_PADDING}/>
          </VBoxLayout>
        </div>
        <div style={EntitlementsPage.STYLE.pagePadding}/>
      </div>);
  }

  private onCommentChange(comment: string) {
    this.setState({comment: comment});  
  }

  private onSubmit() {
    this.props.onSubmit(this.state.comment);
  }

  private static readonly STYLE = {
    page: {
      minWidth: '320px',
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      overflowY: 'auto' as 'auto'
    },
    contentSmall: {
      boxSizing: 'border-box' as 'border-box',
      minWidth: '284px',
      flexShrink: 1,
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      boxSizing: 'border-box' as 'border-box',
      width: '732px'
    },
    contentLarge: {
      boxSizing: 'border-box' as 'border-box',
      width: '1000px'
    },
    pagePadding: {
      width: '18px'
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
