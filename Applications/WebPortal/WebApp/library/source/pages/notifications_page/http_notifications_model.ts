import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalNotificationsModel } from './local_notifications_model';
import { NotificationsFilter, NotificationsModel } from './notifications_model';

/** Implements the NotificationsModel using HTTP requests. */
export class HttpNotificationsModel extends NotificationsModel {

  /** Constructs an HttpNotificationsModel.
   *  @param account - The logged in account.
   *  @param administrationClient - The administration client to use.
   */
  constructor(account: Beam.DirectoryEntry,
      administrationClient: Nexus.AdministrationClient) {
    super();
    this._account = account;
    this._administrationClient = administrationClient;
    this._model = new LocalNotificationsModel();
    this._tasks = new Beam.AsyncWorkQueue();
  }

  public get totalCount(): number {
    return this._model.totalCount;
  }

  public monitorNotifications(
      queue: Beam.QueueWriter<Nexus.Notification>): void {
    this._model.monitorNotifications(queue);
    this._model.loadNotifications(
      Nexus.Notification.ReadState.UNREAD, {
        query: '', categories: new Set(), startDate: null, endDate: null
      }).then((notifications) => {
        for(const notification of notifications) {
          queue.push(notification);
        }
      });
  }

  public async loadNotifications(readStatus: Nexus.Notification.ReadState,
      filter: NotificationsFilter): Promise<Nexus.Notification[]> {
    return this._model.loadNotifications(readStatus, filter);
  }

  public async markAsRead(ids: Nexus.Notification.Id[]): Promise<void> {
    await Promise.all(
      ids.map((id) => this._administrationClient.markNotificationAsRead(id)));
    await this._model.markAsRead(ids);
  }

  public async markAllAsRead(id: Nexus.Notification.Id): Promise<void> {
    var cursor = id;
    while(true) {
      const batch = await this._administrationClient.loadNotifications(
        this._account, cursor, Beam.SnapshotLimit.fromTail(20),
        Nexus.Notification.ReadState.UNREAD);
      if(batch.length === 0) {
        break;
      }
      await Promise.all(batch.map((notification) =>
        this._administrationClient.markNotificationAsRead(notification.id)));
      cursor = batch[0].id;
    }
    await this._model.markAllAsRead(id);
  }

  public async markAsUnread(ids: Nexus.Notification.Id[]): Promise<void> {
    await Promise.all(
      ids.map((id) => this._administrationClient.markNotificationAsUnread(id)));
    await this._model.markAsUnread(ids);
  }

  public async load(): Promise<void> {
    if(this._model.isLoaded) {
      return;
    }
    const notifications = await this._administrationClient.loadNotifications(
      this._account, '', Beam.SnapshotLimit.UNLIMITED,
      Nexus.Notification.ReadState.ALL);
    for(const notification of notifications) {
      this._model.add(notification);
    }
    await this._model.load();
    this._administrationClient.monitorNotifications(this._account,
      this._tasks.getSlot<Nexus.Notification>(
        (notification) => this._model.add(notification)));
  }

  private _account: Beam.DirectoryEntry;
  private _administrationClient: Nexus.AdministrationClient;
  private _model: LocalNotificationsModel;
  private _tasks: Beam.AsyncWorkQueue;
}
