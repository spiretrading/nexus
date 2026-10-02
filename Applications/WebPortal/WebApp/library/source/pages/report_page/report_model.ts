import { CreateReportModel } from './create_report_page';
import { EditScheduledReportModel } from './edit_scheduled_report_page';
import { GeneratedReportsModel } from './generated_reports_page';
import { ReportActivityModel } from './report_activity_page';
import { ScheduledReportsModel } from './scheduled_reports_page';

/** Provides the models for the reporting subpages. */
export abstract class ReportModel {

  /** The model for the Generated Reports Page. */
  public abstract get generatedReportsModel(): GeneratedReportsModel;

  /** The model for the Scheduled Reports Page. */
  public abstract get scheduledReportsModel(): ScheduledReportsModel;

  /** The model for the Report Activity Page. */
  public abstract get reportActivityModel(): ReportActivityModel;

  /** The model for the Create Report Page. */
  public abstract get createReportModel(): CreateReportModel;

  /** The model for editing scheduled reports. */
  public abstract get editScheduledReportModel(): EditScheduledReportModel;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
