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
  }
  return `/notifications/${notification.id}`;
}
