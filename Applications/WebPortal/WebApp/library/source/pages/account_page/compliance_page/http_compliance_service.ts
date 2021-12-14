import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ComplianceModel } from './compliance_model';
import { ComplianceService } from './compliance_service';

/** Implements a ComplianceService over HTTP. */
export class HttpComplianceService extends ComplianceService {

  /**
   * Constructs an HttpComplianceService.
   * @param directoryEntry The account/group represented.
   * @param roles The roles of the account using this service.
   * @param serviceClients The ServiceClients providing the HTTP services.
   */
  constructor(directoryEntry: Beam.DirectoryEntry, roles: Nexus.AccountRoles,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.directoryEntry = directoryEntry;
    this.roles = roles;
    this.serviceClients = serviceClients;
  }

  public async load(): Promise<ComplianceModel> {
    const entries =
      await this.serviceClients.complianceClient.load(this.directoryEntry);
    return new ComplianceModel(this.directoryEntry, this.roles,
      this.serviceClients.definitionsClient.complianceRuleSchemas, entries,
      this.serviceClients.definitionsClient.currencyDatabase);
  }

  public async submit(model: ComplianceModel): Promise<ComplianceModel> {
    for(const entry of model.newEntries) {
      await this.serviceClients.complianceClient.add(this.directoryEntry,
        entry.state, entry.schema);
    }
    for(const entry of model.updatedEntries) {
      await this.serviceClients.complianceClient.update(entry);
    }
    for(const entry of model.deletedEntries) {
      await this.serviceClients.complianceClient.delete(entry.id);
    }
    return await this.load();
  }

  private directoryEntry: Beam.DirectoryEntry;
  private roles: Nexus.AccountRoles;
  private serviceClients: Nexus.ServiceClients;
}
