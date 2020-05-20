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

  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    const groups = await 
      this.serviceClients.administrationClient.loadManagedTradingGroups(
        this.account);
    this._model = new LocalGroupSuggestionModel(groups);
    this._model.load();
  }

  public async loadSuggestions(prefix: string): Promise<Beam.DirectoryEntry[]> {
    return this._model.loadSuggestions(prefix);
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private _isLoaded: boolean;
  private _model: LocalGroupSuggestionModel;
}