import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { VBoxLayout } from 'web_portal';

const buttontest = (
  <WebPortal.VBoxLayout height='100%' width='100%'>
    <WebPortal.Padding />
  <div>
    <WebPortal.EntitlementRow isSecurityActive/>
    <WebPortal.EntitlementRow isSecurityActive/>
  </div>
  <WebPortal.Padding/>
  </WebPortal.VBoxLayout>);

ReactDOM.render(buttontest,
  document.getElementById('main'));
