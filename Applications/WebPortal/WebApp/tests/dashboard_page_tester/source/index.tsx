import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const roles = new Nexus.AccountRoles(0);
const notifications: WebPortal.Notification[] = [
  {id: '1',
    description: 'Your request to update risk controls for achen01 has been approved.',
    category: WebPortal.NotificationCategory.ACCOUNT_MODIFICATION,
    timestamp: (() => {
      const d = new Date(); d.setHours(d.getHours() - 2); return d;
    })(),
    isUnread: true},
  {id: '2',
    description: 'New entitlements request from jberrios01 requires your review.',
    category: WebPortal.NotificationCategory.ACCOUNT_MODIFICATION,
    timestamp: (() => {
      const d = new Date(); d.setDate(d.getDate() - 1); return d;
    })(),
    isUnread: true},
  {id: '3',
    description: 'Risk parameters for trodriguez have been updated.',
    category: WebPortal.NotificationCategory.REPORT,
    timestamp: (() => {
      const d = new Date(); d.setDate(d.getDate() - 3); return d;
    })(),
    isUnread: false}
];

ReactDOM.render(
  <Router.BrowserRouter>
    <WebPortal.DashboardPage roles={roles} notifications={notifications}/>
  </Router.BrowserRouter>,
  document.getElementById('main'));
