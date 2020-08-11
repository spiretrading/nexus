import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { GroupModel } from './group_model';
import { LocalGroupModel } from './local_group_model';

/** Implements the GroupModel using HTTP requests. */
export class HttpGroupModel extends GroupModel {

  /** Constructs an HttpGroupModel.
   * @param group - The group this model represents.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(group: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalGroupModel(group)
    this.serviceClients = serviceClients;
  }

  public get group(): Beam.DirectoryEntry {
    return this.model.group;
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.model.load();
    this.tradingGroup = await this.serviceClients.administrationClient
      .loadTradingGroup(this.model.group);
    this.model = new LocalGroupModel(this.tradingGroup.entry);
    await this.model.load();
  }

  private model: LocalGroupModel;
  private serviceClients: Nexus.ServiceClients;
  private tradingGroup: Nexus.TradingGroup;
}
