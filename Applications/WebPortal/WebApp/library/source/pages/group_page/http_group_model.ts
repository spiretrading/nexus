import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountEntry } from '../account_directory_page';
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
    this.model = new LocalGroupModel(group, []);
    this.serviceClients = serviceClients;
  }

  public get group(): Beam.DirectoryEntry {
    return this.model.group;
  }

  public get accounts(): AccountEntry[] {
    return this._accounts.slice();
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.model.load();
    this.tradingGroup = await this.serviceClients.administrationClient
      .loadTradingGroup(this.model.group);
    const accounts = [] as AccountEntry[];
    for(const manager of this.tradingGroup.managers) {
      const roles = await this.serviceClients.administrationClient
        .loadAccountRoles(manager);
      accounts.push(new AccountEntry(manager, roles));
    }
    for(const trader of this.tradingGroup.traders) {
      const roles = await this.serviceClients.administrationClient
        .loadAccountRoles(trader);
      accounts.push(new AccountEntry(trader, roles));
    }
    this._accounts = accounts.filter(
      (value: AccountEntry, index: number, array: AccountEntry[]) => {
        return array.findIndex((target: AccountEntry) =>
          (target.account.id === value.account.id)) === index});
    this.model = new LocalGroupModel(this.tradingGroup.entry, this._accounts);
    await this.model.load();
  }

  private model: LocalGroupModel;
  private serviceClients: Nexus.ServiceClients;
  private tradingGroup: Nexus.TradingGroup;
  private _accounts: AccountEntry[];
}
