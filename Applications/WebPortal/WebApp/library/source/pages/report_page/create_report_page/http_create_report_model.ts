import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';
import { reportFormValueToJson } from '../report_form_value';
import { CreateReportModel } from './create_report_model';

/** Loads report definitions and submits reports through HTTP. */
export class HttpCreateReportModel extends CreateReportModel {

  /** Constructs a report creation model.
   * @param accountModel - The account and group lookup model.
   * @param scopeModel - The scope lookup model.
   */
  constructor(accountModel: AccountGroupQueryModel,
      scopeModel: QueryModel<Nexus.Scope>) {
    super();
    this.accounts = accountModel;
    this.scopes = scopeModel;
    this.definitions = null;
    this.loading = null;
  }

  /** Returns whether report definitions have been loaded. */
  public get isLoaded(): boolean {
    return this.definitions !== null;
  }

  public get reports(): readonly ReportDefinition[] {
    this.ensureLoaded();
    return this.definitions.slice();
  }

  public get accountModel(): AccountGroupQueryModel {
    this.ensureLoaded();
    return this.accounts;
  }

  public get scopeModel(): QueryModel<Nexus.Scope> {
    this.ensureLoaded();
    return this.scopes;
  }

  public async load(): Promise<void> {
    if(!this.loading) {
      this.loading = Beam.post(
        '/api/reporting_service/load_report_definitions', {}).then(response => {
          this.definitions = response.map(ReportDefinition.fromJson);
        });
    }
    const loading = this.loading;
    try {
      await loading;
    } finally {
      if(this.loading === loading) {
        this.loading = null;
      }
    }
  }

  public async submit(value: ReportFormTemplate.Value): Promise<string> {
    this.ensureLoaded();
    return Beam.post('/api/reporting_service/submit_report',
      reportFormValueToJson(value));
  }

  private ensureLoaded(): void {
    if(!this.definitions) {
      throw new Error('Model not loaded.');
    }
  }

  private accounts: AccountGroupQueryModel;
  private scopes: QueryModel<Nexus.Scope>;
  private definitions: ReportDefinition[];
  private loading: Promise<void>;
}
