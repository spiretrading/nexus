import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { HttpCreateReportModel } from '../create_report_page';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';
import { parseReportFormValue, reportFormValueToJson } from
  '../report_form_value';
import { EditScheduledReportModel } from './edit_scheduled_report_model';

/** Loads and updates scheduled report configurations through HTTP. */
export class HttpEditScheduledReportModel extends EditScheduledReportModel {

  /** Constructs a schedule editing model.
   * @param accountModel - The account and group lookup model.
   * @param scopeModel - The scope lookup model.
   */
  constructor(accountModel: AccountGroupQueryModel,
      scopeModel: QueryModel<Nexus.Scope>) {
    super();
    this.form = new HttpCreateReportModel(accountModel, scopeModel);
  }

  public get reports(): readonly ReportDefinition[] {
    return this.form.reports;
  }

  public get accountModel(): AccountGroupQueryModel {
    return this.form.accountModel;
  }

  public get scopeModel(): QueryModel<Nexus.Scope> {
    return this.form.scopeModel;
  }

  public async load(): Promise<void> {
    await this.form.load();
  }

  public async loadReport(id: string): Promise<ReportFormTemplate.Value> {
    const definitions = this.reports;
    const response = await Beam.post(
      '/api/reporting_service/load_scheduled_report', {id});
    return parseReportFormValue(response, definitions);
  }

  public async submit(id: string, value: ReportFormTemplate.Value):
      Promise<void> {
    if(!this.form.isLoaded) {
      throw new Error('Model not loaded.');
    }
    await Beam.post('/api/reporting_service/update_scheduled_report',
      {id, ...reportFormValueToJson(value)});
  }

  private form: HttpCreateReportModel;
}
