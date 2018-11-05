import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { VBoxLayout } from 'web_portal';
import { Currency } from 'nexus';



const currencyDB = Nexus.buildDefaultCurrencyDatabase();
const cEntry = currencyDB.fromCode('USD');
console.log(cEntry.toJson());
console.log(cEntry.code);
const thing = cEntry.toJson();

const thingy = new Nexus.EntitlementDatabase.Entry('ASX total', 
new Nexus.Money(64), Nexus.DefaultCurrencies.USD, null, null); 




const buttontest = ( 
  <WebPortal.VBoxLayout height='100%' width='100%'>
    <WebPortal.Padding />
  <div>
    <WebPortal.EntitlementRow 
    isSecurityActive
    currencyEntry={cEntry}
    entitlementEntry={thingy}
    />
  </div>
  <WebPortal.Padding/>
  </WebPortal.VBoxLayout>);

ReactDOM.render(buttontest,
  document.getElementById('main'));
