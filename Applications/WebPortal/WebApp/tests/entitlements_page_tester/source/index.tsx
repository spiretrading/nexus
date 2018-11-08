import * as Nexus from 'nexus';
import * as Beam from 'Beam';
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

const test = (
  <WebPortal.HBoxLayout height='100%' width='100%'>
    <WebPortal.Padding size='10%'/>
    <WebPortal.VBoxLayout height='100%' width='80%'>
      <WebPortal.Padding size='10%'/>
      <WebPortal.EntitlementRow
        isActive
        currencyEntry={cEntry1}
        entitlementEntry={entitlementEntry1}
        breakpoint={WebPortal.EntitlementsPageSizing.BreakPoint.SMALL}/>
      <WebPortal.Padding size='40px'/>
      <WebPortal.EntitlementTable
        entitlementEntry={entitlementEntry2}
        breakpoint={WebPortal.EntitlementsPageSizing.BreakPoint.SMALL}
        marketDatabase={marketDB}/>
      <WebPortal.Padding size='40px'/>
      <WebPortal.EntitlementTable
      entitlementEntry={entitlementEntry2}
        breakpoint={WebPortal.EntitlementsPageSizing.BreakPoint.LARGE}
        marketDatabase={marketDB}/>
    </WebPortal.VBoxLayout>
    <WebPortal.Padding size='10%'/>
  </WebPortal.HBoxLayout>);

ReactDOM.render(test,
  document.getElementById('main'));
