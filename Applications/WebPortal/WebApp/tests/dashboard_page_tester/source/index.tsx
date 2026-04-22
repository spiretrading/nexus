import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const roles = new Nexus.AccountRoles(0);
const notifications: Nexus.Notification[] = [
  new Nexus.Notification('1', Beam.DirectoryEntry.INVALID,
    'Your request to update risk controls for achen01 has been approved.',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setHours(d.getHours() - 2); return d;
    })()), false),
  new Nexus.Notification('2', Beam.DirectoryEntry.INVALID,
    'New entitlements request from jberrios01 requires your review.',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 1); return d;
    })()), false),
  new Nexus.Notification('3', Beam.DirectoryEntry.INVALID,
    'Risk parameters for trodriguez have been updated.',
    Nexus.Notification.Category.REPORT,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 3); return d;
    })()), true)
];

ReactDOM.render(
  <Router.BrowserRouter>
    <WebPortal.DashboardPage roles={roles} notifications={notifications}/>
  </Router.BrowserRouter>,
  document.getElementById('main'));
