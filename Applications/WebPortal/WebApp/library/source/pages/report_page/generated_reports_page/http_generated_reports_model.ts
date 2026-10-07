import * as Beam from 'beam';
import { AccountGroupQueryModel } from '../../../components';
import { downloadReport } from '../report_download';
import { GeneratedReportsModel } from './generated_reports_model';

/** Retrieves generated reports and performs report actions through HTTP. */
export class HttpGeneratedReportsModel extends GeneratedReportsModel {

  /** Constructs a generated reports model.
   * @param recipientModel - The account and group lookup model for sharing.
   */
  constructor(recipientModel: AccountGroupQueryModel) {
    super();
    this.recipients = recipientModel;
    this.loaded = false;
  }

  public get recipientModel(): AccountGroupQueryModel {
    this.ensureLoaded();
    return this.recipients;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async query(submission: GeneratedReportsModel.Submission):
      Promise<GeneratedReportsModel.Response> {
    this.ensureLoaded();
    const response = await Beam.post(
      '/api/reporting_service/query_generated_reports', {
        filters: {query: submission.filters.query.trim(),
          start_date: submission.filters.dateRange.start?.toJson() ?? null,
          end_date: submission.filters.dateRange.end?.toJson() ?? null},
        sort: {...submission.sort}, page_index: submission.pageIndex});
    return {status: response.status, isEmpty: response.is_empty,
      filteredCount: response.filtered_count,
      reports: (response.reports ?? []).map((report: any) =>
        ({id: report.id, type: report.type, parameters: [...report.parameters],
          url: report.url,
          dateCreated: Beam.DateTime.fromJson(report.date_created)}))};
  }

  public async delete(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/delete_reports', {ids: [...ids]});
  }

  public async share(ids: readonly string[],
      recipients: readonly Beam.DirectoryEntry[]): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/share_reports',
      {ids: [...ids], recipients: recipients.map(entry => entry.toJson())});
  }

  public async download(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    for(const id of new Set(ids)) {
      await downloadReport(id);
    }
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private recipients: AccountGroupQueryModel;
  private loaded: boolean;
}
