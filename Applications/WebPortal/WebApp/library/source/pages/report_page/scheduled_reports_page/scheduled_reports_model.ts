/** Interface for querying and acting on scheduled reports. */
export abstract class ScheduledReportsModel {

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Queries one page of reports matching the supplied criteria.
   * @param submission - The query and zero-based page index.
   * @return The page and the total number of matching reports.
   */
  public abstract query(submission: ScheduledReportsModel.Submission):
    Promise<ScheduledReportsModel.Response>;

  /** Requests immediate generation of a scheduled report.
   * @param id - The scheduled report's identifier.
   */
  public abstract run(id: string): Promise<void>;

  /** Copies a scheduled report and inserts it after the original.
   * @param id - The scheduled report's identifier.
   * @return The created scheduled report, including its assigned identifier.
   */
  public abstract duplicate(id: string):
    Promise<ScheduledReportsModel.Schedule>;

  /** Deletes a scheduled report.
   * @param id - The scheduled report's identifier.
   */
  public abstract delete(id: string): Promise<void>;
}

export namespace ScheduledReportsModel {

  /** The maximum number of reports returned in one page. */
  export const PAGE_SIZE = 50;

  /** The status of a scheduled reports request. */
  export enum ResponseStatus {

    /** Reports are being retrieved. */
    IN_PROGRESS,

    /** The response is available. */
    READY,

    /** Retrieving reports failed. */
    ERROR
  }

  /** Criteria used to filter scheduled reports. */
  export interface Filters {

    /** The trimmed search query. */
    query: string;
  }

  /** A request to retrieve one page of scheduled reports. */
  export interface Submission {

    /** The filter criteria. */
    filters: Filters;

    /** The zero-based page index. */
    pageIndex: number;
  }

  /** A report parameter and its display value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }

  /** A calendar date and its display label. */
  export interface Date {

    /** The ISO calendar date in YYYY-MM-DD format. */
    value: string;

    /** The localized date label. */
    label: string;
  }

  /** A scheduled report and its formatted display values. */
  export interface Schedule {

    /** The report's unique identifier. */
    id: string;

    /** The report type. */
    type: string;

    /** The report's parameters, in display order. */
    parameters: readonly Parameter[];

    /** Whether the report runs repeatedly. */
    repeats: boolean;

    /** The upcoming run date, or the run date of a one-time report. */
    runDate: Date;
  }

  /** The response for the current submitted criteria and page. */
  export interface Response {

    /** The request status. */
    status: ResponseStatus;

    /** Whether the user has no scheduled reports, irrespective of filters. */
    isEmpty: boolean;

    /** The total number of reports matching the filters across all pages. */
    filteredCount: number;

    /** Up to PAGE_SIZE reports for the requested page, in display order. */
    schedules: readonly Schedule[];
  }
}
