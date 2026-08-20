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
   * Constructs an AccountModificationRequestAnchor positioned before every
   * request.
   * @param id - The id of the request at this position.
   * @param date - The date ordering the request.
   * @param name - The account name ordering the request.
   */
  constructor(id: number = -1,
      date: Beam.DateTime = Beam.DateTime.NEG_INFIN, name: string = '') {
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

  /** Tests if two anchors mark the same position. */
  public equals(other: AccountModificationRequestAnchor): boolean {
    return other && this._id === other._id &&
      this._date.equals(other._date) && this._name === other._name;
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
    this._search = '';
    this._sortField = AccountModificationRequestQuery.SortField.CREATED;
  }

  /** Returns the text matching a request id or an account name. */
  public get search(): string {
    return this._search;
  }

  public set search(value: string) {
    this._search = value;
  }

  /** Returns the field used to order the requests. */
  public get sortField(): AccountModificationRequestQuery.SortField {
    return this._sortField;
  }

  public set sortField(value: AccountModificationRequestQuery.SortField) {
    this._sortField = value;
  }

  /** Tests if two queries specify the same page of requests. */
  public equals(other: AccountModificationRequestQuery): boolean {
    return other && super.equals(other) &&
      this._sortField === other._sortField &&
      this._search === other._search;
  }

  public toString(): string {
    return `(${super.toString()} ${sortFieldToString(this._sortField)})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      ...super.toJson(),
      search: this._search,
      sort_field: this._sortField
    };
  }

  private _search: string;
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

function sortFieldToString(
    field: AccountModificationRequestQuery.SortField): string {
  return AccountModificationRequestQuery.SortField[field];
}

