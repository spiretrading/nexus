import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'Beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const currencyDB = Nexus.buildDefaultCurrencyDatabase();
const marketDB = Nexus.buildDefaultMarketDatabase();

const group =
  new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 89, 'BOOP');
const group2 =
  new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 42, 'MEEP');
const group3 =
  new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 34, 'MEH');
const dataset1 = new Nexus.MarketDataTypeSet(134);
const marketcode1 = new Nexus.MarketCode('XASX');
const ekey1 = new Nexus.EntitlementKey(marketcode1);
const dataset2 = new Nexus.MarketDataTypeSet(1);
const marketcode2 = new Nexus.MarketCode('XCIS');

const ekey2 = new Nexus.EntitlementKey(marketcode2);
const app = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
app.set(ekey1, dataset1);
const app2 = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
app2.set(ekey2, dataset2);

const entitlementEntry1 = new Nexus.EntitlementDatabase.Entry('ASX Total',
  Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group, app);
const cEntry1 = currencyDB.fromCode('USD');
const entitlementEntry2 = new Nexus.EntitlementDatabase.Entry('TSX Venture',
  Nexus.Money.parse('200'), Nexus.DefaultCurrencies.EUR, group2, app2);
const cEntry2 = currencyDB.fromCode('EUR');
const entitlementEntry3 = new Nexus.EntitlementDatabase.Entry('Musk Media',
  Nexus.Money.parse('45'), Nexus.DefaultCurrencies.EUR, group3, app2);

const entitlementDB = new Nexus.EntitlementDatabase();
entitlementDB.add(entitlementEntry1);
entitlementDB.add(entitlementEntry3);
entitlementDB.add(entitlementEntry2);

const roles1 = new Nexus.AccountRoles();
roles1.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
roles1.unset(Nexus.AccountRoles.Role.TRADER);
roles1.unset(Nexus.AccountRoles.Role.MANAGER);
const roles2 = new Nexus.AccountRoles();
roles2.set(Nexus.AccountRoles.Role.TRADER);
roles2.unset(Nexus.AccountRoles.Role.ADMINISTRATOR);
const roles3 = new Nexus.AccountRoles();
roles2.set(Nexus.AccountRoles.Role.MANAGER);

/**  Displays a testing application. */
interface State {
  roles: Nexus.AccountRoles;
  displaySize: WebPortal.DisplaySize;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      roles: roles1,
      displaySize: TestApp.getDisplaySize()
    };
    this.changeRole = this.changeRole.bind(this);
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public render(): JSX.Element {
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.EntitlementsPage
          displaySize={this.state.displaySize}
          marketDatabase={marketDB}
          roles={this.state.roles}
          entitlements={entitlementDB}
          checked={null}
          currencyDatabase={currencyDB}
        />
        <div className={css(TestApp.STYLE.testingComponents)}>
          <button tabIndex={-1}
            onClick={() =>
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
        </div>
      </WebPortal.VBoxLayout>);
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  private changeRole(newRole: Nexus.AccountRoles.Role): void {
    if(newRole === Nexus.AccountRoles.Role.ADMINISTRATOR) {
        this.setState({roles: roles1 });
    }
    if(newRole === Nexus.AccountRoles.Role.TRADER) {
        this.setState({roles: roles2 });
    }
    if(newRole === Nexus.AccountRoles.Role.MANAGER) {
        this.setState({roles: roles3 });
    }
  }

  private static getDisplaySize(): WebPortal.DisplaySize {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return WebPortal.DisplaySize.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return WebPortal.DisplaySize.MEDIUM;
    } else {
      return WebPortal.DisplaySize.LARGE;
    }
  }

  private onScreenResize(): void {
    const newDisplaySize = TestApp.getDisplaySize();
    if(newDisplaySize !== this.state.displaySize) {
      this.setState({ displaySize: newDisplaySize });
    }
  }

  private static STYLE = StyleSheet.create({
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  });
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
