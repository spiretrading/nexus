import * as Beam from 'beam';
import { AccountGroupQueryModel, SortableTableHeaderCell } from
  '../../../components';
import { GeneratedReportsModel } from './generated_reports_model';
import { ReportTable } from './report_table';
import { formatReportDate } from './report_date';

/** Stores generated reports and records share and download requests locally. */
export class LocalGeneratedReportsModel extends GeneratedReportsModel {

  /** Constructs a local model.
   * @param reports - The initial generated reports, in default order.
   * @param recipientModel - The account and group lookup model.
   */
  constructor(reports: readonly ReportTable.Report[],
      recipientModel: AccountGroupQueryModel) {
    super();
    this.loaded = false;
    this.entries = new Map(reports.map(report => [report.id, copy(report)]));
    this.recipients = recipientModel;
    this.shareRequests = [];
    this.downloadRequests = [];
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Returns copies of the generated reports in default order. */
  public get reports(): readonly ReportTable.Report[] {
    this.ensureLoaded();
    return Array.from(this.entries.values(), copy);
  }

  /** Returns snapshots of requested shares. */
  public get shares(): readonly LocalGeneratedReportsModel.Share[] {
    this.ensureLoaded();
    return this.shareRequests.map(request => ({ids: [...request.ids],
      recipients: [...request.recipients]}));
  }

  /** Returns the report identifiers in each download request. */
  public get downloads(): readonly (readonly string[])[] {
    this.ensureLoaded();
    return this.downloadRequests.map(ids => [...ids]);
  }

  public get recipientModel(): AccountGroupQueryModel {
    this.ensureLoaded();
    return this.recipients;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async loadReports(submission: GeneratedReportsModel.Submission):
      Promise<GeneratedReportsModel.Response> {
    this.ensureLoaded();
    const {query, dateRange} = submission.filters;
    const matches = Array.from(this.entries.values()).filter(report => {
      return (!dateRange.start ||
          report.dateCreated.compare(dateRange.start) >= 0) &&
        (!dateRange.end || report.dateCreated.compare(dateRange.end) <= 0) &&
        [report.type, ...report.parameters,
          formatReportDate(report.dateCreated)].
          some(text => text.toLowerCase().includes(query.toLowerCase()));
    });
    if(submission.sort.order !== SortableTableHeaderCell.SortOrder.NONE) {
      matches.sort((left, right) => {
        const result = (() => {
          switch(submission.sort.column) {
            case ReportTable.Column.TYPE:
              return left.type.localeCompare(right.type);
            case ReportTable.Column.PARAMETERS:
              return left.parameters.join(' \u2022 ').localeCompare(
                right.parameters.join(' \u2022 '));
            case ReportTable.Column.DATE_CREATED:
              return left.dateCreated.compare(right.dateCreated);
          }
        })();
        if(submission.sort.order ===
            SortableTableHeaderCell.SortOrder.DESCENDING) {
          return -result;
        }
        return result;
      });
    }
    const start = submission.pageIndex * GeneratedReportsModel.PAGE_SIZE;
    return {status: GeneratedReportsModel.ResponseStatus.READY,
      isEmpty: this.entries.size === 0, filteredCount: matches.length,
      reports: matches.slice(start, start + GeneratedReportsModel.PAGE_SIZE).
        map(copy)};
  }

  public async delete(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    for(const id of ids) {
      this.entries.delete(id);
    }
  }

  public async share(ids: readonly string[],
      recipients: readonly Beam.DirectoryEntry[]): Promise<void> {
    this.ensureLoaded();
    this.shareRequests.push({ids: [...ids], recipients: [...recipients]});
  }

  public async download(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    this.downloadRequests.push([...ids]);
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
  private entries: Map<string, ReportTable.Report>;
  private recipients: AccountGroupQueryModel;
  private shareRequests: LocalGeneratedReportsModel.Share[];
  private downloadRequests: string[][];
}

export namespace LocalGeneratedReportsModel {

  /** A recorded request to share generated reports. */
  export interface Share {

    /** The identifiers of the reports to share. */
    ids: readonly string[];

    /** The accounts and groups receiving the reports. */
    recipients: readonly Beam.DirectoryEntry[];
  }
}

function copy(report: ReportTable.Report): ReportTable.Report {
  return {...report, parameters: [...report.parameters],
    dateCreated: new Beam.Date(report.dateCreated.year,
      report.dateCreated.month, report.dateCreated.day)};
}
