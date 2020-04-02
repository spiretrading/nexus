import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalRiskModel } from './local_risk_model';
import { RiskModel } from './risk_model';

/** Implements a RiskModel using HTTP requests. */
export class HttpRiskModel extends RiskModel {

  /** Constructs an HttpRiskModel.
   * @param account - The account to represent.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalRiskModel(account, Nexus.RiskParameters.INVALID);
    this.serviceClients = serviceClients;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get riskParameters(): Nexus.RiskParameters {
    return this.model.riskParameters;
  }

  public async load(): Promise<void> {
    await this.model.load();
    const parameters =
      await this.serviceClients.administrationClient.loadRiskParameters(
      this.account);
    this.model = new LocalRiskModel(this.account, parameters);
    await this.model.load();
  }

  public async submit(comment: string,
      riskParameters: Nexus.RiskParameters): Promise<void> {
    const modification = new Nexus.RiskModification(riskParameters);
    await this.serviceClients.administrationClient.
      submitRiskModificationRequest(this.account, modification,
      Nexus.Message.fromPlainText(comment));
  }

  private model: LocalRiskModel;
  private serviceClients: Nexus.ServiceClients;
}
