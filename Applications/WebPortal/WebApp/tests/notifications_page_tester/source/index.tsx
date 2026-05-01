import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const DESCRIPTIONS = [
  {description: 'Risk parameters have been updated.',
    category: Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    status: 'GRANTED'},
  {description: 'Entitlements have been updated.',
    category: Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    status: 'GRANTED'},
  {description: 'Entitlement modification request has been rejected.',
    category: Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    status: 'REJECTED'},
  {description: 'Risk modification request has been rejected.',
    category: Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    status: 'REJECTED'},
  {description: 'Monthly compliance report is available.',
    category: Nexus.Notification.Category.REPORT,
    status: ''},
  {description: 'Quarterly risk assessment report is ready.',
    category: Nexus.Notification.Category.REPORT,
    status: ''}
];

function generateNotifications(count: number): Nexus.Notification[] {
  const now = new Date();
  const todayStart = new Date(
    now.getFullYear(), now.getMonth(), now.getDate());
  const spanMs = (count / 10) * 24 * 60 * 60 * 1000;
  const notifications: Nexus.Notification[] = [];
  for(let i = 0; i < count; ++i) {
    const t = i / Math.max(count - 1, 1);
    const timestamp = new Date(now.getTime() - t * spanMs);
    const isToday = timestamp >= todayStart;
    const isRead = isToday ? false : Math.random() >= 0.01;
    const entry = DESCRIPTIONS[i % DESCRIPTIONS.length];
    const requestId = 100 + i;
    const data = entry.status ?
      `{"request_id":${requestId},"status":"${entry.status}"}` : '';
    notifications.push(new Nexus.Notification(
      String(i + 1), Beam.DirectoryEntry.INVALID,
      entry.description, data, entry.category,
      Beam.DateTime.fromDate(timestamp), isRead));
  }
  return notifications;
}

const model = new WebPortal.LocalNotificationsModel(
  generateNotifications(200));

function App(): JSX.Element {
  return (
    <div style={STYLE.wrapper}>
      <Router.BrowserRouter>
        <Router.Route render={(props) =>
          <WebPortal.NotificationsController {...props} model={model}/>}/>
      </Router.BrowserRouter>
    </div>);
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  }
};

model.load().then(() => {
  ReactDOM.render(<App/>, document.getElementById('main'));
});
