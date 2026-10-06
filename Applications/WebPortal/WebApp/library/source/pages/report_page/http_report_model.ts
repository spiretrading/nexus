import * as Nexus from 'nexus';
import { HttpAccountGroupQueryModel } from '../../components';
import { HttpTickerQueryModel, ScopeQueryModel } from '../../models';
import { CompositeReportModel } from './composite_report_model';
import { HttpCreateReportModel } from './create_report_page';
import { HttpEditScheduledReportModel } from './edit_scheduled_report_page';
import { HttpGeneratedReportsModel } from './generated_reports_page';
import { HttpReportActivityModel } from './report_activity_page';
import { HttpReportDetailModel } from './report_detail_page';
import { HttpScheduledReportsModel } from './scheduled_reports_page';

/** Provides HTTP implementations of the reporting subpage models. */
export class HttpReportModel extends CompositeReportModel {

  /** Constructs the report models.
   * @param serviceClients - The clients for account and group lookups.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    const accounts =
      new HttpAccountGroupQueryModel(serviceClients.administrationClient);
    const scopes = new ScopeQueryModel(new HttpTickerQueryModel());
    super(new HttpScheduledReportsModel(),
      new HttpGeneratedReportsModel(accounts), new HttpReportActivityModel(),
      new HttpCreateReportModel(accounts, scopes),
      new HttpEditScheduledReportModel(accounts, scopes),
      new HttpReportDetailModel());
    this.serviceClients = serviceClients;
  }

  public async load(): Promise<void> {
    await this.serviceClients.open();
    await super.load();
  }

  private serviceClients: Nexus.ServiceClients;
}
