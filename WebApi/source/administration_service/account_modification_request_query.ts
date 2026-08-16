import * as Beam from 'beam';
import { AccountModificationRequest } from './account_modification_request';

/** Specifies a page of account modification requests to load. */
export class AccountModificationRequestQuery {

  /**
   * Constructs an AccountModificationRequestQuery.
   * @param index - The account or directory whose requests are to be loaded.
   * @param limit - The maximum number of requests to load.
   */
  constructor(index: Beam.DirectoryEntry, limit: number) {
    this.index = index;
    this.limit = limit;
    this.isHead = false;
    this.anchor = null;
    this.categories = [];
    this.startDate = null;
    this.endDate = null;
    this.query = '';
    this.excludedAccount = null;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      index: this.index.toJson(),
      anchor: toOptionalJson(this.anchor),
      limit: this.limit,
      is_head: this.isHead,
      categories: this.categories.slice(),
      start_date: toOptionalJson(this.startDate?.toJson()),
      end_date: toOptionalJson(this.endDate?.toJson()),
      query: this.query,
      excluded_account: toOptionalJson(this.excludedAccount?.toJson())
    };
  }

  /** The account or directory whose requests are to be loaded. */
  public index: Beam.DirectoryEntry;

  /** The id to page from, or null to start from the beginning or end. */
  public anchor: number;

  /** The maximum number of requests to load. */
  public limit: number;

  /** Whether to load the requests following the anchor. */
  public isHead: boolean;

  /** The request categories to match, or empty to match all. */
  public categories: AccountModificationRequest.Type[];

  /** The earliest timestamp to match. */
  public startDate: Beam.DateTime;

  /** The latest timestamp to match. */
  public endDate: Beam.DateTime;

  /** Matches a request id or an account name. */
  public query: string;

  /** An account whose requests are to be excluded. */
  public excludedAccount: Beam.DirectoryEntry;
}

function toOptionalJson(value: any): any {
  if(value === null || value === undefined) {
    return {is_initialized: false};
  }
  return {is_initialized: true, value: value};
}
