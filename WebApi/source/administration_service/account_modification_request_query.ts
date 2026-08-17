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

  /** The request categories to match, or empty to match all. */
  public categories: AccountModificationRequest.Type[];

  /** The earliest timestamp to match. */
  public startDate: Beam.DateTime;

  /** The latest timestamp to match. */
  public endDate: Beam.DateTime;

  /** Matches a request id or an account name. */
  public search: string;

  /** An account whose requests are to be excluded. */
  public excludedAccount: Beam.DirectoryEntry;

  /** The field used to order the requests. */
  public sortField: AccountModificationRequestQuery.SortField;

  /**
   * Constructs an AccountModificationRequestQuery.
   * @param index - The account or directory whose requests are to be loaded.
   * @param snapshotLimit - The limit on the page of requests to load.
   */
  constructor(index: Beam.DirectoryEntry, snapshotLimit: Beam.SnapshotLimit) {
    super(index);
    this.snapshotLimit = snapshotLimit;
    this.categories = [];
    this.startDate = null;
    this.endDate = null;
    this.search = '';
    this.excludedAccount = null;
    this.sortField = AccountModificationRequestQuery.SortField.CREATED;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      ...super.toJson(),
      categories: this.categories.slice(),
      start_date: toOptionalJson(this.startDate?.toJson()),
      end_date: toOptionalJson(this.endDate?.toJson()),
      search: this.search,
      excluded_account: toOptionalJson(this.excludedAccount?.toJson()),
      sort_field: this.sortField
    };
  }
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

function toOptionalJson(value: any): any {
  if(value === null || value === undefined) {
    return {is_initialized: false};
  }
  return {is_initialized: true, value: value};
}
