import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';

/** Loads and updates the typed configuration of a scheduled report. */
export abstract class EditScheduledReportModel {

  /** The report types available to the current user. */
  public abstract get reports(): readonly ReportDefinition[];

  /** The account and group lookup model. */
  public abstract get accountModel(): AccountGroupQueryModel;

  /** The scope lookup model. */
  public abstract get scopeModel(): QueryModel<Nexus.Scope>;

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Retrieves a scheduled report's editable configuration. */
  public abstract loadReport(id: string): Promise<ReportFormTemplate.Value>;

  /** Saves the configuration of the identified scheduled report. */
  public abstract submit(id: string, value: ReportFormTemplate.Value):
    Promise<void>;
}
