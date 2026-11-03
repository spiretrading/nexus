import { CreateReportModel } from './create_report_page';
import { EditScheduledReportModel } from './edit_scheduled_report_page';
import { GeneratedReportsModel } from './generated_reports_page';
import { ReportDetailModel } from './report_detail_page';
import { ReportModel } from './report_model';
import { ScheduledReportsModel } from './scheduled_reports_page';

/** Composes caller-supplied reporting subpage models. */
export class CompositeReportModel extends ReportModel {

  /**
   * Constructs a report model.
   * @param scheduledReportsModel - The model for scheduled reports.
   * @param generatedReportsModel - The model for generated reports.
   * @param createReportModel - The model for creating reports.
   * @param editScheduledReportModel - The model for editing schedules.
   * @param reportDetailModel - The model for generated report details.
   */
  constructor(scheduledReportsModel: ScheduledReportsModel,
      generatedReportsModel: GeneratedReportsModel,
      createReportModel: CreateReportModel,
      editScheduledReportModel: EditScheduledReportModel,
      reportDetailModel: ReportDetailModel) {
    super();
    this.loaded = false;
    this.scheduledReports = scheduledReportsModel;
    this.generatedReports = generatedReportsModel;
    this.createReport = createReportModel;
    this.editScheduledReport = editScheduledReportModel;
    this.reportDetail = reportDetailModel;
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

  public get createReportModel(): CreateReportModel {
    this.ensureLoaded();
    return this.createReport;
  }

  public get editScheduledReportModel(): EditScheduledReportModel {
    this.ensureLoaded();
    return this.editScheduledReport;
  }

  public get reportDetailModel(): ReportDetailModel {
    this.ensureLoaded();
    return this.reportDetail;
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
  private createReport: CreateReportModel;
  private editScheduledReport: EditScheduledReportModel;
  private reportDetail: ReportDetailModel;
}
