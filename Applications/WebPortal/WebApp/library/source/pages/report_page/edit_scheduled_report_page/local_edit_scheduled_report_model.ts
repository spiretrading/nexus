import * as Nexus from 'nexus';
import { AccountGroupQueryModel } from '../../../components';
import { QueryModel } from '../../../models';
import { ReportDefinition } from '../report_definition';
import { ReportFormTemplate } from '../report_form_template';
import { copyReportFormValue } from '../report_form_value';
import { EditScheduledReportModel } from './edit_scheduled_report_model';

/** Stores editable scheduled report configurations in memory. */
export class LocalEditScheduledReportModel extends EditScheduledReportModel {

  /** Constructs a local model.
   * @param reports - The permitted report definitions.
   * @param accountModel - The account and group lookup model.
   * @param scopeModel - The scope lookup model.
   * @param schedules - The initial configurations keyed by schedule ID.
   */
  constructor(reports: readonly ReportDefinition[],
      accountModel: AccountGroupQueryModel, scopeModel: QueryModel<Nexus.Scope>,
      schedules: ReadonlyMap<string, ReportFormTemplate.Value>) {
    super();
    this.loaded = false;
    this.definitions = reports.slice();
    this.accounts = accountModel;
    this.scopes = scopeModel;
    this.entries = new Map(Array.from(schedules,
      ([id, value]) => [id, copyReportFormValue(value, this.definitions)]));
  }

  /** Returns whether the model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Adds or replaces a local schedule's configuration. */
  public set(id: string, value: ReportFormTemplate.Value): void {
    this.ensureLoaded();
    this.entries.set(id, copyReportFormValue(value, this.definitions));
  }

  /** Removes a local schedule's configuration. */
  public delete(id: string): void {
    this.ensureLoaded();
    this.entries.delete(id);
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

  public async loadReport(id: string): Promise<ReportFormTemplate.Value> {
    this.ensureLoaded();
    const value = this.entries.get(id);
    if(!value) {
      throw new Error(`Scheduled report not found: ${id}`);
    }
    return copyReportFormValue(value, this.definitions);
  }

  public async submit(id: string, value: ReportFormTemplate.Value):
      Promise<void> {
    this.ensureLoaded();
    if(!this.entries.has(id)) {
      throw new Error(`Scheduled report not found: ${id}`);
    }
    this.set(id, value);
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
  private entries: Map<string, ReportFormTemplate.Value>;
}
