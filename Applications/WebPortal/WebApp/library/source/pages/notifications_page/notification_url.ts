import * as Nexus from 'nexus';

/** Returns the URL to navigate to when a notification is clicked. */
export function getNotificationUrl(notification: Nexus.Notification): string {
  if(notification.category === Nexus.Notification.Category.ACCOUNT_MODIFICATION) {
    try {
      const data = JSON.parse(notification.data);
      if(data.request_id != null) {
        return `/request_history/${data.request_id}`;
      }
    } catch {}
  } else if(notification.category === Nexus.Notification.Category.REPORT) {
    try {
      const data = JSON.parse(notification.data);
      if(typeof data.report_id === 'string' && data.report_id !== '') {
        return `/reports/${encodeURIComponent(data.report_id)}`;
      }
    } catch {}
  }
  return `/notifications/${notification.id}`;
}
