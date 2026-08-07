import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** The filter criteria for notifications. */
export interface NotificationsFilter {

  /** The filter query. */
  query: string;

  /** The set of selected categories. */
  categories: Set<Nexus.Notification.Category>;

  /** The selected start date. */
  startDate: Beam.Date;

  /** The selected end date. */
  endDate: Beam.Date;
}

/** Model used by the NotificationsController. */
export abstract class NotificationsModel {

  /** The total number of notifications the account has. */
  public abstract get totalCount(): number;

  /** Monitors notifications for the logged in account.
   *  @param queue - The queue to push notifications onto.
   */
  public abstract monitorNotifications(
    queue: Beam.QueueWriter<Nexus.Notification>): void;

  /** Loads notifications matching the given criteria.
   *  @param readStatus - The read status filter.
   *  @param filter - The filter criteria.
   *  @return The list of matching notifications.
   */
  public abstract loadNotifications(readStatus: Nexus.Notification.ReadState,
    filter: NotificationsFilter): Promise<Nexus.Notification[]>;

  /** Marks notifications as read.
   *  @param ids - The ids of the notifications to mark as read.
   */
  public abstract markAsRead(ids: Nexus.Notification.Id[]): Promise<void>;

  /** Marks all unread notifications up to and including the given id as read.
   *  @param id - The id of the oldest notification to mark as read.
   */
  public abstract markAllAsRead(id: Nexus.Notification.Id): Promise<void>;

  /** Marks notifications as unread.
   *  @param ids - The ids of the notifications to mark as unread.
   */
  public abstract markAsUnread(ids: Nexus.Notification.Id[]): Promise<void>;

  /** Loads/refreshes the model's data from the server. */
  public abstract load(): Promise<void>;
}
