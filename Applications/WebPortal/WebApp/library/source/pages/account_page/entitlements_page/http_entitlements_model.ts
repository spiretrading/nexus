import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { EntitlementsModel } from './entitlements_model';
import { LocalEntitlementsModel } from './local_entitlements_model';

/** Implements an EntitlementsModel by using HTTP requests. */
export class HttpEntitlementsModel extends EntitlementsModel {

  /** Constructs an HttpEntitlementsModel.
   * @param account - The account to represent.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalEntitlementsModel(account,
      new Beam.Set<Beam.DirectoryEntry>());
    this.serviceClients = serviceClients;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get entitlements(): Beam.Set<Beam.DirectoryEntry> {
    return this.model.entitlements;
  }

  public async load(): Promise<void> {
    await this.model.load();
    const entitlements =
      await this.serviceClients.administrationClient.loadAccountEntitlements(
      this.account);
    this.model = new LocalEntitlementsModel(this.account, entitlements);
    await this.model.load();
  }

  public async submit(comment: string,
      entitlements: Beam.Set<Beam.DirectoryEntry>): Promise<void> {
    const modification = new Nexus.EntitlementModification(entitlements);
    await this.serviceClients.administrationClient.
      submitEntitlementModificationRequest(this.account, modification,
      Nexus.Message.fromPlainText(comment));
  }

  private model: LocalEntitlementsModel;
  private serviceClients: Nexus.ServiceClients;
}
