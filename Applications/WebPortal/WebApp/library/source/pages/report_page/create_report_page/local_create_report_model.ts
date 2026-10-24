import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';
import { copyReportFormValue } from '../report_form_value';
import { CreateReportModel } from './create_report_model';

/** Provides local report definitions and stores submitted configurations. */
export class LocalCreateReportModel extends CreateReportModel {

  /** Constructs a local model.
   * @param reports - The permitted report definitions.
   * @param accountModel - The account and group lookup model.
   * @param scopeModel - The scope lookup model.
   */
  constructor(reports: readonly ReportDefinition[],
      accountModel: AccountGroupQueryModel,
      scopeModel: QueryModel<Nexus.Scope>) {
    super();
    this.loaded = false;
    this.definitions = reports.slice();
    this.accounts = accountModel;
    this.scopes = scopeModel;
    this.submissions = [];
  }

  /** Returns whether the model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Returns copies of the submitted configurations in submission order. */
  public get requests(): readonly ReportFormTemplate.Value[] {
    this.ensureLoaded();
    return this.submissions.map(
      value => copyReportFormValue(value, this.definitions));
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
    this.loaded = true;
  }

  public async submit(value: ReportFormTemplate.Value): Promise<string> {
    this.ensureLoaded();
    this.submissions.push(copyReportFormValue(value, this.definitions));
    return String(this.submissions.length);
  }

  private ensureLoaded(): void {
    if(!this.loaded) {
      throw new Error('Model not loaded.');
    }
  }

  private loaded: boolean;
  private definitions: readonly ReportDefinition[];
  private accounts: AccountGroupQueryModel;
  private scopes: QueryModel<Nexus.Scope>;
  private submissions: ReportFormTemplate.Value[];
}
