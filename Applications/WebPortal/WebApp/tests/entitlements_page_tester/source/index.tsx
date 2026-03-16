import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

class TestEntitlementsModel extends WebPortal.LocalEntitlementsModel {
  public shouldFail = false;

  public async submit(comment: string,
      entitlements: Beam.Set<Beam.DirectoryEntry>,
      effectiveDate: Beam.Date): Promise<void> {
    if(this.shouldFail) {
      throw Error('Server issue');
    }
  }
}

interface State {
  roles: Nexus.AccountRoles;
  shouldFail: boolean;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    const roles = new Nexus.AccountRoles();
    roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    this.state = {
      roles,
      shouldFail: false
    };
  }

  public render(): JSX.Element {
    const isAdmin =
      this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR);
    return (
      <div style={STYLE.wrapper}>
        <WebPortal.EntitlementsController
          roles={this.state.roles}
          model={this.model}
          entitlements={this.entitlementDB}
          currencyDatabase={this.currencyDB}
          venueDatabase={this.venueDB}
          displaySize={WebPortal.DisplaySize.LARGE}/>
        <div style={STYLE.toolbar}>
          <label style={STYLE.toggle}>
            <input type='checkbox' checked={isAdmin}
              onChange={this.onToggleAdmin}/>
            Admin
          </label>
          <label style={STYLE.toggle}>
            <input type='checkbox' checked={this.state.shouldFail}
              onChange={this.onToggleError}/>
            Error
          </label>
        </div>
      </div>);
  }

  private onToggleAdmin = () => {
    const roles = this.state.roles.clone();
    if(roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      roles.unset(Nexus.AccountRoles.Role.ADMINISTRATOR);
    } else {
      roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    }
    this.setState({roles});
  }

  private onToggleError = () => {
    this.model.shouldFail = !this.state.shouldFail;
    this.setState({shouldFail: !this.state.shouldFail});
  }

  private model = (() => {
    const checked = new Beam.Set<Beam.DirectoryEntry>();
    checked.add(
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 89, 'BOOP'));
    return new TestEntitlementsModel(
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 123, 'test'),
      checked);
  })();

  private entitlementDB = (() => {
    const group1 =
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
    const app1 = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
    app1.set(ekey1, dataset1);
    app1.set(ekey2, dataset2);
    const app2 = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
    app2.set(ekey2, dataset2);
    const db = new Nexus.EntitlementDatabase();
    db.add(new Nexus.EntitlementDatabase.Entry('ASX Total',
      Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group1, app1));
    db.add(new Nexus.EntitlementDatabase.Entry('TSX CDG',
      Nexus.Money.parse('45'), Nexus.DefaultCurrencies.EUR, group3, app2));
    db.add(new Nexus.EntitlementDatabase.Entry('TSX Venture',
      Nexus.Money.parse('200'), Nexus.DefaultCurrencies.EUR, group2, app2));
    db.add(new Nexus.EntitlementDatabase.Entry('TSX TL1',
      Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group4, app1));
    db.add(new Nexus.EntitlementDatabase.Entry('CSE',
      Nexus.Money.parse('175'), Nexus.DefaultCurrencies.USD, group5, app1));
    db.add(new Nexus.EntitlementDatabase.Entry('Alpha APD',
      Nexus.Money.parse('0'), Nexus.DefaultCurrencies.USD, group6, app1));
    return db;
  })();

  private currencyDB = Nexus.buildDefaultCurrencyDatabase();
  private venueDB = Nexus.buildDefaultVenueDatabase();
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  },
  toolbar: {
    display: 'flex',
    flexDirection: 'row',
    gap: '10px',
    position: 'absolute'
  },
  toggle: {
    display: 'flex',
    alignItems: 'center',
    gap: '4px',
    cursor: 'pointer'
  }
};

ReactDOM.render(<TestApp/>, document.getElementById('main'));
