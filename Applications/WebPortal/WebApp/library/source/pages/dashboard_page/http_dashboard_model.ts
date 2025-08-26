import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel, AccountEntry, HttpAccountDirectoryModel,
  HttpAccountModel, HttpGroupModel, LocalAccountDirectoryModel } from '..';
import { DashboardModel } from './dashboard_model';
import { LocalDashboardModel } from './local_dashboard_model';

/** Implements the DashboardModel using HTTP requests. */
export class HttpDashboardModel extends DashboardModel {

  /** Constructs an HttpDashboardModel.
   * @param serviceClients - The service clients used to access the HTTP
   *        services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
    this.accountModels = new Beam.Map<Beam.DirectoryEntry, HttpAccountModel>();
    this.groupModels = new Beam.Map<Beam.DirectoryEntry, HttpGroupModel>();
    this.model = new LocalDashboardModel(Beam.DirectoryEntry.INVALID,
      new Nexus.AccountRoles(0), new Nexus.EntitlementDatabase(),
      new Nexus.CountryDatabase(), new Nexus.CurrencyDatabase(),
      new Nexus.VenueDatabase(), new LocalAccountDirectoryModel(
      new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>()));
  }

  public get entitlementDatabase(): Nexus.EntitlementDatabase {
    return this.model.entitlementDatabase;
  }

  public get countryDatabase(): Nexus.CountryDatabase {
    return this.model.countryDatabase;
  }

  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this.model.currencyDatabase;
  }

  public get venueDatabase(): Nexus.VenueDatabase {
    return this.model.venueDatabase;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public get accountDirectoryModel(): AccountDirectoryModel {
    return this.model.accountDirectoryModel;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): HttpAccountModel {
    let model = this.accountModels.get(account);
    if(model === undefined) {
      model = new HttpAccountModel(account, this.serviceClients);
      this.accountModels.set(account, model);
    }
    return model;
  }

  public makeGroupModel(group: Beam.DirectoryEntry): HttpGroupModel {
    let model = this.groupModels.get(group);
    if(model === undefined) {
      model = new HttpGroupModel(group, this.serviceClients);
      this.groupModels.set(group, model);
    }
    return model;
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.serviceClients.open();
    const account = await
      this.serviceClients.serviceLocatorClient.loadCurrentAccount();
    const roles = await
      this.serviceClients.administrationClient.loadAccountRoles(account);
    this.model = new LocalDashboardModel(account, roles,
      this.serviceClients.definitionsClient.entitlementDatabase,
      this.serviceClients.definitionsClient.countryDatabase,
      this.serviceClients.definitionsClient.currencyDatabase,
      this.serviceClients.definitionsClient.venueDatabase,
      new HttpAccountDirectoryModel(account, this.serviceClients));
    return this.model.load();
  }

  public async logout(): Promise<void> {
    this.serviceClients.close();
  }

  private serviceClients: Nexus.ServiceClients;
  private accountModels: Beam.Map<Beam.DirectoryEntry, HttpAccountModel>;
  private groupModels: Beam.Map<Beam.DirectoryEntry, HttpGroupModel>;
  private model: LocalDashboardModel;
}
