import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'Beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const currencyDB = Nexus.buildDefaultCurrencyDatabase();
const marketDB = Nexus.buildDefaultMarketDatabase();

const group =
  new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 89, 'BOOP');
const dataset1 = new Nexus.MarketDataTypeSet(134);
const marketcode1 = new Nexus.MarketCode('XASX');
const ekey1 = new Nexus.EntitlementKey(marketcode1);
const dataset2 = new Nexus.MarketDataTypeSet(1);
const marketcode2 = new Nexus.MarketCode('XCIS');
const ekey2 = new Nexus.EntitlementKey(marketcode2);
const app = new Beam.Map<Nexus.EntitlementKey, Nexus.MarketDataTypeSet>();
app.set(ekey1, dataset1);
app.set(ekey2, dataset2);

const entitlementEntry1 = new Nexus.EntitlementDatabase.Entry('ASX Total',
  Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, group, app);
const cEntry1 = currencyDB.fromCode('USD');
const entitlementEntry2 = new Nexus.EntitlementDatabase.Entry('TSX Venture',
  Nexus.Money.parse('200'), Nexus.DefaultCurrencies.EUR, group, app);
const cEntry2 = currencyDB.fromCode('EUR');
const entitlementEntry3 = new Nexus.EntitlementDatabase.Entry('Musk Media',
  Nexus.Money.parse('45'), Nexus.DefaultCurrencies.EUR, group, app);

const entitlementDB = new Nexus.EntitlementDatabase();
const input = [entitlementEntry1, entitlementEntry2, entitlementEntry3];
entitlementDB.add(entitlementEntry1);
entitlementDB.add(entitlementEntry3);
entitlementDB.add(entitlementEntry2);
//why does add not work more than once?



const test = (
  <WebPortal.HBoxLayout height='100%' width='100%'>
      <WebPortal.EntitlementsPage
        displaySize={WebPortal.DisplaySize.SMALL}
        marketDatabase={marketDB}
        roles={null}
        entitlements={entitlementDB}
        checked={null}
        currencyDatabase={currencyDB}
        />
  </WebPortal.HBoxLayout>);

ReactDOM.render(test,
  document.getElementById('main'));

/**  Displays a testing application for the login page. */


interface State {
  roles: Nexus.AccountRoles.Role; //hmmmmmmmmmmmmmmm
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      roles : Nexus.AccountRoles.Role.ADMINISTRATOR
    };
    this.changeRole = this.changeRole.bind(this);
  }

  public render(): JSX.Element {
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <div className={css(TestApp.STYLE.testingComponents)}>
        <button tabIndex={-1}
        onClick={() => this.changeRole(Nexus.AccountRoles.Role.ADMINISTRATOR)}>
          ADMINISTRATOR
        </button>
        <button tabIndex={-1}
        onClick={() => this.changeRole(Nexus.AccountRoles.Role.TRADER)}>
          TRADER
        </button>
        </div>
      </WebPortal.VBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  });

  private changeRole(newRole: Nexus.AccountRoles.Role): void {
    this.setState({
      roles: newRole
    });
  }
}
ReactDOM.render(<TestApp/>, document.getElementById('main'));
