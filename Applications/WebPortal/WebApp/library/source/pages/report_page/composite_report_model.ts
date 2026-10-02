import { GeneratedReportsModel } from './generated_reports_page';
import { ReportActivityModel } from './report_activity_page';
import { ReportModel } from './report_model';
import { ScheduledReportsModel } from './scheduled_reports_page';

/** Composes caller-supplied reporting subpage models. */
export class CompositeReportModel extends ReportModel {

  /** Constructs a report model.
   * @param scheduledReportsModel - The model for scheduled reports.
   * @param generatedReportsModel - The model for generated reports.
   * @param reportActivityModel - The model for report jobs.
   */
  constructor(scheduledReportsModel: ScheduledReportsModel,
      generatedReportsModel: GeneratedReportsModel,
      reportActivityModel: ReportActivityModel) {
    super();
    this.loaded = false;
    this.scheduledReports = scheduledReportsModel;
    this.generatedReports = generatedReportsModel;
    this.reportActivity = reportActivityModel;
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  public get scheduledReportsModel(): ScheduledReportsModel {
    this.ensureLoaded();
    return this.scheduledReports;
  }

  public get generatedReportsModel(): GeneratedReportsModel {
    this.ensureLoaded();
    return this.generatedReports;
  }

  public get reportActivityModel(): ReportActivityModel {
    this.ensureLoaded();
    return this.reportActivity;
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
  private generatedReports: GeneratedReportsModel;
  private reportActivity: ReportActivityModel;
}
