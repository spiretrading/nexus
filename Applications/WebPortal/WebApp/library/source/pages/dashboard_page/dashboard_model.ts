import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel, AccountModel, GroupModel } from '..';
import { RequestsModel } from '../requests_page/requests_model';

/** Base class for the model used by the DashboardPage. */
export abstract class DashboardModel {

  /** Returns the database of entitlements. */
  public abstract get entitlementDatabase(): Nexus.EntitlementDatabase;

  /** Returns the database of countries. */
  public abstract get countryDatabase(): Nexus.CountryDatabase;

  /** Returns the database of currencies. */
  public abstract get currencyDatabase(): Nexus.CurrencyDatabase;

  /** Returns the database of venues. */
  public abstract get venueDatabase(): Nexus.VenueDatabase;

  /** The account currently logged in. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Returns the AccountDirectoryModel. */
  public abstract get accountDirectoryModel(): AccountDirectoryModel;

  /** Returns the RequestsModel. */
  public abstract get requestsModel(): RequestsModel;

  /**
   * Monitors notifications for the logged in account.
   * @param queue - The queue to push notifications onto.
   */
  public abstract monitorNotifications(
    queue: Beam.QueueWriter<Nexus.Notification>): void;

  /**
   * Marks a notification as read.
   * @param id - The id of the notification to mark as read.
   */
  public abstract markNotificationAsRead(
    id: Nexus.Notification.Id): Promise<void>;

  /** Makes a new account model. */
  public abstract makeAccountModel(account: Beam.DirectoryEntry): AccountModel;

  /** Makes a new group model. */
  public abstract makeGroupModel(group: Beam.DirectoryEntry): GroupModel;

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Logs the user out. */
  public abstract logout(): Promise<void>;
}
