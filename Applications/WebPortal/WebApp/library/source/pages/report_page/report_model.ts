import { ScheduledReportsModel } from './scheduled_reports_page';

/** Provides the models for the reporting subpages. */
export abstract class ReportModel {

  /** The model for the Scheduled Reports Page. */
  public abstract get scheduledReportsModel(): ScheduledReportsModel;

  /** Loads this model. */
  public abstract load(): Promise<void>;
}
