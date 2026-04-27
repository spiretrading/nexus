import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const now = new Date();

function hoursAgo(hours: number): Beam.DateTime {
  const d = new Date(now);
  d.setHours(d.getHours() - hours);
  return Beam.DateTime.fromDate(d);
}

function daysAgo(days: number): Beam.DateTime {
  const d = new Date(now);
  d.setDate(d.getDate() - days);
  return Beam.DateTime.fromDate(d);
}

const NOTIFICATIONS: Nexus.Notification[] = [
  new Nexus.Notification('1', Beam.DirectoryEntry.INVALID,
    'Risk parameters have been updated.', '{"request_id":101,"status":"GRANTED"}',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION, hoursAgo(1), false),
  new Nexus.Notification('2', Beam.DirectoryEntry.INVALID,
    'Entitlements have been updated.', '{"request_id":102,"status":"GRANTED"}',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION, hoursAgo(3), false),
  new Nexus.Notification('3', Beam.DirectoryEntry.INVALID,
    'Entitlement modification request has been rejected.',
    '{"request_id":103,"status":"REJECTED"}',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION, daysAgo(1), false),
  new Nexus.Notification('4', Beam.DirectoryEntry.INVALID,
    'Risk modification request has been rejected.',
    '{"request_id":104,"status":"REJECTED"}',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION, daysAgo(2), true),
  new Nexus.Notification('5', Beam.DirectoryEntry.INVALID,
    'Monthly compliance report is available.', '',
    Nexus.Notification.Category.REPORT, daysAgo(3), true),
  new Nexus.Notification('6', Beam.DirectoryEntry.INVALID,
    'Risk parameters have been updated.', '{"request_id":105,"status":"GRANTED"}',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION, daysAgo(5), true),
  new Nexus.Notification('7', Beam.DirectoryEntry.INVALID,
    'Quarterly risk assessment report is ready.', '',
    Nexus.Notification.Category.REPORT, daysAgo(7), true)
];

const model = new WebPortal.LocalNotificationsModel(NOTIFICATIONS);

function App(): JSX.Element {
  return (
    <Router.BrowserRouter>
      <Router.Route render={(props) =>
        <WebPortal.NotificationsController {...props} model={model}/>}/>
    </Router.BrowserRouter>);
}

model.load().then(() => {
  ReactDOM.render(<App/>, document.getElementById('main'));
});
