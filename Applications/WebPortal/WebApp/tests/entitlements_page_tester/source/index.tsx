import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  roles: Nexus.AccountRoles;
  checkedDB: Beam.Set<Beam.DirectoryEntry>;
  currencyDB: Nexus.CurrencyDatabase;
  venueDB: Nexus.VenueDatabase;
  entitlementDB: Nexus.EntitlementDatabase;
  status: string;
  displayedStatus: string;
  submitEnabled: boolean;
}

/**  Displays and tests the CreateAccountPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      roles:  new Nexus.AccountRoles(),
      entitlementDB: new Nexus.EntitlementDatabase(),
      checkedDB: new Beam.Set<Beam.DirectoryEntry>(),
      currencyDB: Nexus.buildDefaultCurrencyDatabase(),
      venueDB: Nexus.buildDefaultVenueDatabase(),
      status: '',
      displayedStatus: '',
      submitEnabled: false
    };
  }

  public render(): JSX.Element {
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.EntitlementsPage displaySize={this.props.displaySize}
          venueDatabase={this.state.venueDB} roles={this.state.roles}
          entitlements={this.state.entitlementDB} checked={this.state.checkedDB}
          currencyDatabase={this.state.currencyDB}
          onEntitlementClick={this.toggleCheckMark}
          status={this.state.displayedStatus}
          canSubmit={this.state.submitEnabled}
          onSubmit={this.commentsSubmitted}/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1} onClick={() =>
              this.changeRole(Nexus.AccountRoles.Role.ADMINISTRATOR)}>
            ADMINISTRATOR
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.TRADER)}>
            TRADER
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.MANAGER)}>
            MANAGER
          </button>
          <button tabIndex={-1} onClick={() => this.changeStatus('')}>
            NOT SUBMITTED
          </button>
          <button tabIndex={-1} onClick={() => this.changeStatus('Saved')}>
            SUCCESSFUL SUBMIT
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeStatus('Server issue')}>
            UNSUCCESSFUL SUBMIT
          </button>
          <button tabIndex={-1} onClick={this.toggleButtonEnabled}>
            TOGGLE SUBMIT
          </button>
        </div>
      </Dali.VBoxLayout>);
  }

  public componentDidMount(): void {
    this.setState({ roles: this.testAdmin });
    this.testAdmin.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    this.testTrader.set(Nexus.AccountRoles.Role.TRADER);
    this.testManager.set(Nexus.AccountRoles.Role.MANAGER);
    this.buildEntitlementDB();
  }

  private changeRole(newRole: Nexus.AccountRoles.Role): void {
    if(newRole === Nexus.AccountRoles.Role.ADMINISTRATOR) {
      this.setState({ roles: this.testAdmin });
    } else if(newRole === Nexus.AccountRoles.Role.TRADER) {
      this.setState({ roles: this.testTrader });
    } else if(newRole === Nexus.AccountRoles.Role.MANAGER) {
      this.setState({ roles: this.testManager });
    }
  }

  private changeStatus(newStatus: string) {
    if(newStatus === '') {
      this.setState({displayedStatus: ''});
    } else if(newStatus !== this.state.status) {
      this.setState({ status: newStatus});
    }
  }

  private commentsSubmitted = (value: string) => {
    this.setState({ displayedStatus: this.state.status.toString()});
  }

  private toggleButtonEnabled = () => {
    this.setState(state => {submitEnabled: !state.submitEnabled});
  }

  private toggleCheckMark = (value: Beam.DirectoryEntry) => {
    if(!this.state.checkedDB.test(value)) {
      this.state.checkedDB.add(value);
    } else {
      this.state.checkedDB.remove(value);
    }
    this.setState({checkedDB: this.state.checkedDB});
  }

  private buildEntitlementDB() {
    const group =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 89, 'BOOP');
    const group2 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 42, 'MEEP');
    const group3 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 34, 'MEH');
    const group4 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 35, 'MEH');
    const group5 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 36, 'MEH');
    const group6 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 37, 'MEH');

    const dataset1 = new Nexus.MarketDataTypeSet(134);
    const venuecode1 = new Nexus.Venue('XASX');
    const ekey1 = new Nexus.EntitlementKey(venuecode1);
    const dataset2 = new Nexus.MarketDataTypeSet(1);
    const venuecode2 = new Nexus.Venue('XCIS');
    const ekey2 = new Nexus.EntitlementKey(venuecode2);

    const app = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
    app.set(ekey1, dataset1);
    app.set(ekey2, dataset2);
    const app2 = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
    app2.set(ekey2, dataset2);

    const entitlementEntry1 = new Nexus.EntitlementDatabase.Entry('ASX Total',
      Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group, app);
    const entitlementEntry2 = new Nexus.EntitlementDatabase.Entry('TSX Venture',
      Nexus.Money.parse('200'), Nexus.DefaultCurrencies.EUR, group2, app2);
    const entitlementEntry3 = new Nexus.EntitlementDatabase.Entry('TSX CDG',
      Nexus.Money.parse('45'), Nexus.DefaultCurrencies.EUR, group3, app2);
    const entitlementEntry4 = new Nexus.EntitlementDatabase.Entry('TSX TL1',
      Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group4, app);
    const entitlementEntry5 = new Nexus.EntitlementDatabase.Entry('CSE',
      Nexus.Money.parse('175'), Nexus.DefaultCurrencies.USD, group5, app);
    const entitlementEntry6 = new Nexus.EntitlementDatabase.Entry('Alpha APD',
      Nexus.Money.parse('0'), Nexus.DefaultCurrencies.USD, group6, app);

    this.state.entitlementDB.add(entitlementEntry1);
    this.state.entitlementDB.add(entitlementEntry3);
    this.state.entitlementDB.add(entitlementEntry2);
    this.state.entitlementDB.add(entitlementEntry4);
    this.state.entitlementDB.add(entitlementEntry5);
    this.state.entitlementDB.add(entitlementEntry6);

    this.state.checkedDB.add(group);
  }

  private testAdmin = new Nexus.AccountRoles();
  private testTrader = new Nexus.AccountRoles();
  private testManager = new Nexus.AccountRoles();
  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage />, document.getElementById('main'));
