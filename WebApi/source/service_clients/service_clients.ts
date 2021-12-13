import * as Beam from 'beam';
import {AdministrationClient, DefinitionsClient, RiskClient} from '..';

/** Client used to access Nexus services. */
export abstract class ServiceClients {

  /** Returns the ServiceLocatorClient. */
  public abstract get serviceLocatorClient(): Beam.ServiceLocatorClient;

  /** Returns the AdministrationClient. */
  public abstract get administrationClient(): AdministrationClient;

  /** Returns the DefinitionsClient. */
  public abstract get definitionsClient(): DefinitionsClient;

  /** Returns the RiskClient. */
  public abstract get riskClient(): RiskClient;

  /**
   * Logs into and opens all services.
   * @param username - The account's username.
   * @param password - The account's password.
   * @return The directory entry of the account that logged in.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract login(username: string, password: string):
    Promise<Beam.DirectoryEntry>;

  /** Connects to all services, by-passing login.*/
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
