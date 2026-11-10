import * as Beam from 'beam';
import { ReportDetailModel } from './report_detail_model';

/** Retrieves report details and optional text content through HTTP. */
export class HttpReportDetailModel extends ReportDetailModel {
  constructor() {
    super();
    this.loaded = false;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async loadReport(id: string): Promise<ReportDetailModel.Report> {
    this.ensureLoaded();
    const response = await Beam.post(
      '/api/reporting_service/load_report', {id});
    return {id: response.id, title: response.title, status: response.status,
      parameters: response.parameters.map((parameter: any) =>
        ({label: parameter.label, value: parameter.value})),
      mediaType: response.media_type, filePath: response.file_path,
      content: response.content};
  }

  public async retry(id: string): Promise<void> {
    this.ensureLoaded();
    await Beam.post('/api/reporting_service/retry_report_jobs', {ids: [id]});
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
}
