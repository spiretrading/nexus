import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalGroupSuggestionModel } from './local_group_suggestion_model';
import { GroupSuggestionModel } from './group_suggestion_model';

export class HttpGroupSuggestionModel extends GroupSuggestionModel {

  /** Constructs an HttpGroupSuggestionModel.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.account = account;
    this.serviceClients = serviceClients;
    this._model = new LocalGroupSuggestionModel([]);
  }

  public async load(): Promise<void> {
    if(!this._model.isLoaded) {
      const groups = await 
        this.serviceClients.administrationClient.loadManagedTradingGroups(
        this.account);
      this._model = new LocalGroupSuggestionModel(groups);
      await this._model.load();
    }
  }

  public async loadSuggestions(prefix: string): Promise<Beam.DirectoryEntry[]> {
    return this._model.loadSuggestions(prefix);
  }

  public async addGroup(directory: Beam.DirectoryEntry): Promise<void> {
    await this._model.addGroup(directory);
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private _model: LocalGroupSuggestionModel;
}
