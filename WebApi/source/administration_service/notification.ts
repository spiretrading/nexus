import * as Beam from 'beam';

/** Stores a notification sent to an account. */
export class Notification {

  /** Constructs a Notification from a JSON object. */
  public static fromJson(value: any): Notification {
    return new Notification(value.id,
      Beam.DirectoryEntry.fromJson(value.account), value.description,
      value.category, Beam.DateTime.fromJson(value.timestamp), value.is_read);
  }

  /**
   * Constructs a Notification.
   * @param id - The unique identifier for the notification.
   * @param account - The account that the notification is for.
   * @param description - The description of the notification.
   * @param category - The category of the notification.
   * @param timestamp - The timestamp when the notification was created.
   * @param isRead - Whether the notification has been read.
   */
  constructor(id: string = '', account: Beam.DirectoryEntry =
      Beam.DirectoryEntry.INVALID, description: string = '',
      category: Notification.Category = Notification.Category.ACCOUNT_MODIFICATION,
      timestamp: Beam.DateTime = Beam.DateTime.NOT_A_DATE_TIME,
      isRead: boolean = false) {
    this._id = id;
    this._account = account;
    this._description = description;
    this._category = category;
    this._timestamp = timestamp;
    this._isRead = isRead;
  }

  /** Returns the unique identifier for the notification. */
  public get id(): string {
    return this._id;
  }

  /** Returns the account that the notification is for. */
  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  /** Returns the description of the notification. */
  public get description(): string {
    return this._description;
  }

  /** Returns the category of the notification. */
  public get category(): Notification.Category {
    return this._category;
  }

  /** Returns the timestamp when the notification was created. */
  public get timestamp(): Beam.DateTime {
    return this._timestamp;
  }

  /** Returns whether the notification has been read. */
  public get isRead(): boolean {
    return this._isRead;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      account: this._account.toJson(),
      description: this._description,
      category: this._category,
      timestamp: this._timestamp.toJson(),
      is_read: this._isRead
    };
  }

  private _id: string;
  private _account: Beam.DirectoryEntry;
  private _description: string;
  private _category: Notification.Category;
  private _timestamp: Beam.DateTime;
  private _isRead: boolean;
}

export module Notification {

  /** Lists the categories of notifications. */
  export enum Category {
    ACCOUNT_MODIFICATION = 0,
    REPORT
  }

  /** Filters notifications by read state. */
  export enum ReadState {
    UNREAD = 1,
    READ = 2,
    ALL = 3
  }
}
