/** The category of a notification. */
export enum NotificationCategory {

  /** The notification pertains to an account modification. */
  ACCOUNT_MODIFICATION = 'ACCOUNT_MODIFICATION',

  /** The notification pertains to a report. */
  REPORT = 'REPORT'
}

/** A single notification entry. */
export interface Notification {

  /** The unique identifier for the notification. */
  id: string;

  /** The description of the notification. */
  description: string;

  /** The category of the notification. */
  category: NotificationCategory;

  /** The datetime when the notification was created. */
  timestamp: Date;

  /** Whether the notification is unread. */
  isUnread: boolean;
}
