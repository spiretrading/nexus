import { ReportDetailModel } from './report_detail_model';

/** Stores generated report metadata and text content in memory. */
export class LocalReportDetailModel extends ReportDetailModel {

  /** Constructs a local model containing the supplied reports. */
  constructor(reports: readonly ReportDetailModel.Report[]) {
    super();
    this.loaded = false;
    this.entries =
      new Map(reports.map(report => [report.id, copyReport(report)]));
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Removes a generated report from the local model. */
  public delete(id: string): void {
    this.ensureLoaded();
    this.entries.delete(id);
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async loadReport(id: string): Promise<ReportDetailModel.Report> {
    this.ensureLoaded();
    const report = this.entries.get(id);
    if(!report) {
      throw new Error(`Report not found: ${id}`);
    }
    return copyReport(report);
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
  private entries: Map<string, ReportDetailModel.Report>;
}

function copyReport(report: ReportDetailModel.Report):
    ReportDetailModel.Report {
  return {...report, parameters: report.parameters.map(parameter =>
    ({...parameter}))};
}
