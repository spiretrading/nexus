import { SortableTableHeaderCell } from '../../../components';
import { ActivityTable } from './activity_table';

/** Retrieves report jobs and performs actions on them. */
export abstract class ReportActivityModel {

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Retrieves one page of activity and the total count. */
  public abstract loadActivities(submission: ReportActivityModel.Submission):
      Promise<ReportActivityModel.Response>;

  /** Cancels the identified report jobs. */
  public abstract cancel(ids: readonly string[]): Promise<void>;

  /** Retries the identified failed report jobs. */
  public abstract retry(ids: readonly string[]): Promise<void>;
}

export namespace ReportActivityModel {

  /** The maximum number of report jobs returned in one page. */
  export const PAGE_SIZE = 50;

  /** The status of a request for report activity. */
  export enum ResponseStatus {

    /** Report jobs are being retrieved. */
    IN_PROGRESS,

    /** The response is available. */
    READY,

    /** Retrieving report jobs failed. */
    ERROR
  }

  /** The requested ordering of report jobs. */
  export interface Sort {

    /** The column to sort. */
    column: ActivityTable.Column;

    /** The direction, or NONE for the model's default ordering. */
    order: SortableTableHeaderCell.SortOrder;
  }

  /** A request for one page of report jobs. */
  export interface Submission {

    /** The requested ordering. */
    sort: Sort;

    /** The zero-based page index. */
    pageIndex: number;
  }

  /** The response for a requested page. */
  export interface Response {

    /** The request status. */
    status: ResponseStatus;

    /** Whether there are no report jobs across all pages. */
    isEmpty: boolean;

    /** The total number of report jobs across all pages. */
    totalCount: number;

    /** Up to PAGE_SIZE report jobs for the requested page. */
    activities: readonly ActivityTable.Activity[];
  }
}
