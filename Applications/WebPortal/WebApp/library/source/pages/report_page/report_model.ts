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

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
