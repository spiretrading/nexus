import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const currencyDB = Nexus.buildDefaultCurrencyDatabase();
const entitlementEntry1 = new Nexus.EntitlementDatabase.Entry('ASX Total', 
Nexus.Money.parse('68'), Nexus.DefaultCurrencies.USD, null, null);
const cEntry1 = currencyDB.fromCode('USD'); 
const entitlementEntry2 = new Nexus.EntitlementDatabase.Entry('TSX Venture', 
Nexus.Money.parse('200'), Nexus.DefaultCurrencies.EUR, null, null);
const cEntry2 = currencyDB.fromCode('EUR'); 


const buttontest = (
  <WebPortal.HBoxLayout height='100%' width='100%'>
    <WebPortal.Padding size='10%'/> 
    <WebPortal.VBoxLayout height='100%' width='80%'>
      <WebPortal.Padding />
      <div>
        <WebPortal.EntitlementRow 
        isSecurityActive
        currencyEntry={cEntry1}
        entitlementEntry={entitlementEntry1}/>
      </div>
      <div>
        <WebPortal.EntitlementRow 
        isSecurityActive={false}
        currencyEntry={cEntry2}
        entitlementEntry={entitlementEntry2}/>
      </div>
      <WebPortal.Padding/>
    </WebPortal.VBoxLayout>
    <WebPortal.Padding size='10%'/> 
  </WebPortal.HBoxLayout>);

ReactDOM.render(buttontest,
  document.getElementById('main'));
