import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { NotificationsFilter, NotificationsModel } from './notifications_model';

/** Implements the NotificationsModel using local memory. */
export class LocalNotificationsModel extends NotificationsModel {

  /** Constructs a LocalNotificationsModel.
   *  @param notifications - The initial set of notifications.
   */
  constructor(notifications: Nexus.Notification[] = []) {
    super();
    this._isLoaded = false;
    this._notifications = [];
    this._publisher = new Beam.QueueWriterPublisher<Nexus.Notification>();
    for(const notification of notifications) {
      this.add(notification);
    }
  }

  /** Returns true if this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  /** Adds a notification in sorted order by timestamp.
   *  @param notification - The notification to add.
   */
  public add(notification: Nexus.Notification): void {
    const existing = this._notifications.findIndex(
      (n) => n.id === notification.id);
    if(existing !== -1) {
      this._notifications[existing] = notification;
      this._publisher.push(notification);
      return;
    }
    const last = this._notifications[this._notifications.length - 1];
    if(!last || last.timestamp.compare(notification.timestamp) >= 0) {
      this._notifications.push(notification);
    } else {
      const index = this._notifications.findIndex(
        (n) => n.timestamp.compare(notification.timestamp) <= 0);
      this._notifications.splice(index, 0, notification);
    }
    this._publisher.push(notification);
  }

  public get totalCount(): number {
    this.ensureLoaded();
    return this._notifications.length;
  }

  public monitorNotifications(
      queue: Beam.QueueWriter<Nexus.Notification>): void {
    this.ensureLoaded();
    this._publisher.monitor(queue);
  }

  public async loadNotifications(readStatus: Nexus.Notification.ReadState,
      filter: NotificationsFilter): Promise<Nexus.Notification[]> {
    this.ensureLoaded();
    return this._notifications.filter((n) => {
      if(readStatus === Nexus.Notification.ReadState.UNREAD && n.isRead) {
        return false;
      }
      if(readStatus === Nexus.Notification.ReadState.READ && !n.isRead) {
        return false;
      }
      if(filter.categories.size > 0 && !filter.categories.has(n.category)) {
        return false;
      }
      if(filter.query && !n.description.toLowerCase().includes(
          filter.query.toLowerCase())) {
        return false;
      }
      const date = Beam.Date.fromDate(n.timestamp.toDate());
      if(filter.startDate && date.compare(filter.startDate) < 0) {
        return false;
      }
      if(filter.endDate && date.compare(filter.endDate) > 0) {
        return false;
      }
      return true;
    });
  }

  public async markAsRead(ids: Nexus.Notification.Id[]): Promise<void> {
    this.ensureLoaded();
    const idSet = new Set(ids);
    for(let i = 0; i < this._notifications.length; ++i) {
      const n = this._notifications[i];
      if(idSet.has(n.id) && !n.isRead) {
        const updated = new Nexus.Notification(
          n.id, n.account, n.description, n.data, n.category, n.timestamp,
          true);
        this._notifications[i] = updated;
        this._publisher.push(updated);
      }
    }
  }

  public async markAllAsRead(id: Nexus.Notification.Id): Promise<void> {
    this.ensureLoaded();
    const index = this._notifications.findIndex((n) => n.id === id);
    if(index === -1) {
      return;
    }
    const ids = this._notifications.
      slice(0, index + 1).
      filter((n) => !n.isRead).
      map((n) => n.id);
    await this.markAsRead(ids);
  }

  public async markAsUnread(ids: Nexus.Notification.Id[]): Promise<void> {
    this.ensureLoaded();
    const idSet = new Set(ids);
    for(let i = 0; i < this._notifications.length; ++i) {
      const n = this._notifications[i];
      if(idSet.has(n.id) && n.isRead) {
        const updated = new Nexus.Notification(n.id, n.account, n.description,
          n.data, n.category, n.timestamp, false);
        this._notifications[i] = updated;
        this._publisher.push(updated);
      }
    }
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private ensureLoaded() {
    if(!this._isLoaded) {
      throw Error('Model not loaded.');
    }
  }

  private _isLoaded: boolean;
  private _notifications: Nexus.Notification[];
  private _publisher: Beam.QueueWriterPublisher<Nexus.Notification>;
}
