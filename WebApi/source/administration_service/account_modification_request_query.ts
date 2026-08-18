import * as Beam from 'beam';
import { AccountModificationRequest } from './account_modification_request';

/**
 * Marks a position within an ordering of AccountModificationRequests. The
 * field used depends on the ordering being paged.
 */
export class AccountModificationRequestAnchor {

  /** Constructs an AccountModificationRequestAnchor from a JSON object. */
  public static fromJson(value: any): AccountModificationRequestAnchor {
    return new AccountModificationRequestAnchor(
      value.id, Beam.DateTime.fromJson(value.date), value.name);
  }

  /**
   * Constructs an AccountModificationRequestAnchor.
   * @param id - The id of the request at this position.
   * @param date - The date ordering the request.
   * @param name - The account name ordering the request.
   */
  constructor(id: number, date: Beam.DateTime, name: string) {
    this._id = id;
    this._date = date;
    this._name = name;
  }

  /** Returns the id of the request at this position. */
  public get id(): number {
    return this._id;
  }

  /** Returns the date ordering the request. */
  public get date(): Beam.DateTime {
    return this._date;
  }

  /** Returns the account name ordering the request. */
  public get name(): string {
    return this._name;
  }

  public toString(): string {
    return `(${this._id} ${this._date} ${this._name})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      date: this._date.toJson(),
      name: this._name
    };
  }

  private _id: number;
  private _date: Beam.DateTime;
  private _name: string;
}

/** Specifies a page of account modification requests to load. */
export class AccountModificationRequestQuery extends
    Beam.PagedQuery<Beam.DirectoryEntry, AccountModificationRequestAnchor> {

  /**
   * Constructs an AccountModificationRequestQuery.
   * @param index - The account or directory whose requests are to be loaded.
   * @param snapshotLimit - The limit on the page of requests to load.
   */
  constructor(index: Beam.DirectoryEntry, snapshotLimit: Beam.SnapshotLimit) {
    super(index);
    this.snapshotLimit = snapshotLimit;
    this._categories = [];
    this._statuses = [];
    this._startDate = null;
    this._endDate = null;
    this._search = '';
    this._excludedAccount = null;
    this._sortField = AccountModificationRequestQuery.SortField.CREATED;
  }

  /** Returns the request categories to match, or empty to match all. */
  public get categories(): AccountModificationRequest.Type[] {
    return this._categories;
  }

  public set categories(value: AccountModificationRequest.Type[]) {
    this._categories = value;
  }

  /** Returns the request statuses to match, or empty to match all. */
  public get statuses(): AccountModificationRequest.Status[] {
    return this._statuses;
  }

  public set statuses(value: AccountModificationRequest.Status[]) {
    this._statuses = value;
  }

  /** Returns the earliest timestamp to match. */
  public get startDate(): Beam.DateTime {
    return this._startDate;
  }

  public set startDate(value: Beam.DateTime) {
    this._startDate = value;
  }

  /** Returns the latest timestamp to match. */
  public get endDate(): Beam.DateTime {
    return this._endDate;
  }

  public set endDate(value: Beam.DateTime) {
    this._endDate = value;
  }

  /** Returns the text matching a request id or an account name. */
  public get search(): string {
    return this._search;
  }

  public set search(value: string) {
    this._search = value;
  }

  /** Returns the account whose requests are to be excluded. */
  public get excludedAccount(): Beam.DirectoryEntry {
    return this._excludedAccount;
  }

  public set excludedAccount(value: Beam.DirectoryEntry) {
    this._excludedAccount = value;
  }

  /** Returns the field used to order the requests. */
  public get sortField(): AccountModificationRequestQuery.SortField {
    return this._sortField;
  }

  public set sortField(value: AccountModificationRequestQuery.SortField) {
    this._sortField = value;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      ...super.toJson(),
      categories: this._categories.slice(),
      statuses: this._statuses.slice(),
      start_date: toOptionalJson(this._startDate?.toJson()),
      end_date: toOptionalJson(this._endDate?.toJson()),
      search: this._search,
      excluded_account: toOptionalJson(this._excludedAccount?.toJson()),
      sort_field: this._sortField
    };
  }

  private _categories: AccountModificationRequest.Type[];
  private _statuses: AccountModificationRequest.Status[];
  private _startDate: Beam.DateTime;
  private _endDate: Beam.DateTime;
  private _search: string;
  private _excludedAccount: Beam.DirectoryEntry;
  private _sortField: AccountModificationRequestQuery.SortField;
}

export namespace AccountModificationRequestQuery {

  /** Lists the fields that requests can be ordered by. */
  export enum SortField {

    /** Order by the time a request was submitted. */
    CREATED = 0,

    /** Order by the time a request was last updated. */
    LAST_UPDATED = 1,

    /** Order by the date a request takes effect. */
    EFFECTIVE_DATE = 2,

    /** Order by the name of the account being modified. */
    ACCOUNT = 3,

    /** Order by the name of the account that submitted the request. */
    REQUESTER = 4
  }
}

/** Converts an optional value to its JSON representation. */
export function toOptionalJson(value: any): any {
  if(value === null || value === undefined) {
    return {is_initialized: false};
  }
  return {is_initialized: true, value: value};
}
