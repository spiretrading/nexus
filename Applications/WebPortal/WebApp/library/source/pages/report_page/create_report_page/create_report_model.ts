import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';

/** Provides available reports and submits new report configurations. */
export abstract class CreateReportModel {

  /** The permitted report definitions after loading. */
  public abstract get reports(): readonly ReportDefinition[];

  /** The model used to look up accounts and groups after loading. */
  public abstract get accountModel(): AccountGroupQueryModel;

  /** The model used to look up scope entries after loading. */
  public abstract get scopeModel(): QueryModel<Nexus.Scope>;

  /** Loads the report definitions and input models. */
  public abstract load(): Promise<void>;

  /** Creates a report job or schedule and returns its identifier. */
  public abstract submit(value: ReportFormTemplate.Value): Promise<string>;
}
