import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel using HTTP requests. */
export class HttpAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an HttpAccountDirectoryModel.
   * @param serviceClients - The ServiceClients used to query.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
  }

  public async load(): Promise<void> {}

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    return new Beam.Set<Beam.DirectoryEntry>();
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    return [];
  }

  public async loadFilteredAccounts(
      filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    return new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
  }

  private serviceClients: Nexus.ServiceClients;
}
