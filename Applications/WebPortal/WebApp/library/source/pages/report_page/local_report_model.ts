import { ReportModel } from './report_model';
import { ScheduledReportsModel } from './scheduled_reports_page';

/** Composes reporting subpage models locally. */
export class LocalReportModel extends ReportModel {

  /** Constructs a report model.
   * @param scheduledReportsModel - The model for scheduled reports.
   */
  constructor(scheduledReportsModel: ScheduledReportsModel) {
    super();
    this.loaded = false;
    this.scheduledReports = scheduledReportsModel;
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  public get scheduledReportsModel(): ScheduledReportsModel {
    this.ensureLoaded();
    return this.scheduledReports;
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
  private scheduledReports: ScheduledReportsModel;
}
