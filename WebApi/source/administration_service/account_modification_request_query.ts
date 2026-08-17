import * as Beam from 'beam';
import { AccountModificationRequest } from './account_modification_request';

/** Specifies a page of account modification requests to load. */
export class AccountModificationRequestQuery extends
    Beam.PagedQuery<Beam.DirectoryEntry, number> {

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
    LAST_UPDATED = 1
  }
}

function toOptionalJson(value: any): any {
  if(value === null || value === undefined) {
    return {is_initialized: false};
  }
  return {is_initialized: true, value: value};
}
