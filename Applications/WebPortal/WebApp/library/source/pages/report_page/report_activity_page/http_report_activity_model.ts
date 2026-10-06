import * as Beam from 'beam';
import { ReportActivityModel } from './report_activity_model';

/** Retrieves report activity and performs job actions through HTTP. */
export class HttpReportActivityModel extends ReportActivityModel {
  constructor() {
    super();
    this.loaded = false;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async query(submission: ReportActivityModel.Submission):
      Promise<ReportActivityModel.Response> {
    this.ensureLoaded();
    const response = await Beam.post(
      '/api/reporting_service/query_report_activities',
      {sort: {...submission.sort}, page_index: submission.pageIndex});
    return {status: response.status, isEmpty: response.is_empty,
      totalCount: response.total_count,
      activities: (response.activities ?? []).map((entry: any) =>
        ({id: entry.id, type: entry.type, parameters: [...entry.parameters],
          status: entry.status,
          dateModified: Beam.Date.fromJson(entry.date_modified)}))};
  }

  public async cancel(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/cancel_report_jobs',
      {ids: [...ids]});
  }

  public async retry(ids: readonly string[]): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/retry_report_jobs',
      {ids: [...ids]});
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
}
