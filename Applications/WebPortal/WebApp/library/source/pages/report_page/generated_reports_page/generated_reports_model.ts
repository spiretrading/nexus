import * as Beam from 'beam';
import { AccountGroupQueryModel, SortableTableHeaderCell } from
  '../../../components';
import { DateRange } from '../../../models/date_range';
import { ReportTable } from './report_table';

/** Loads generated reports and performs actions on selected reports. */
export abstract class GeneratedReportsModel {

  /** The model used to look up share recipients after loading. */
  public abstract get recipientModel(): AccountGroupQueryModel;

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Retrieves one page and the total count for the supplied criteria. */
  public abstract loadReports(submission: GeneratedReportsModel.Submission):
      Promise<GeneratedReportsModel.Response>;

  /** Deletes the reports with the supplied identifiers. */
  public abstract delete(ids: readonly string[]): Promise<void>;

  /** Shares the identified reports with the supplied accounts and groups. */
  public abstract share(ids: readonly string[],
    recipients: readonly Beam.DirectoryEntry[]): Promise<void>;

  /** Downloads the identified reports. */
  public abstract download(ids: readonly string[]): Promise<void>;
}

export namespace GeneratedReportsModel {

  /** The maximum number of reports returned in one page. */
  export const PAGE_SIZE = 50;

  /** The status of a generated reports request. */
  export enum ResponseStatus {

    /** Reports are being retrieved. */
    IN_PROGRESS,

    /** The response is available. */
    READY,

    /** Retrieving reports failed. */
    ERROR
  }

  /** Criteria used to filter generated reports. */
  export interface Filters {

    /** The trimmed search query. */
    query: string;

    /** Inclusive bounds on creation dates; null bounds are unbounded. */
    dateRange: DateRange;
  }

  /** The requested ordering of generated reports. */
  export interface Sort {

    /** The column to sort. */
    column: ReportTable.Column;

    /** The sort direction, or NONE for the model's default ordering. */
    order: SortableTableHeaderCell.SortOrder;
  }

  /** A request to retrieve one page of generated reports. */
  export interface Submission {

    /** The filter criteria. */
    filters: Filters;

    /** The requested ordering. */
    sort: Sort;

    /** The zero-based page index. */
    pageIndex: number;
  }

  /** A response for the submitted criteria and page. */
  export interface Response {

    /** The request status. */
    status: ResponseStatus;

    /** Whether there are no generated reports, irrespective of filters. */
    isEmpty: boolean;

    /** The total number of matching reports across all pages. */
    filteredCount: number;

    /** Up to PAGE_SIZE reports for the requested page. */
    reports: readonly ReportTable.Report[];
  }
}
