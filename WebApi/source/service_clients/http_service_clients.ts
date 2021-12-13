import * as Beam from 'beam';
import { HttpAdministrationClient, HttpComplianceClient, HttpDefinitionsClient,
  HttpRiskClient } from '..';
import { ServiceClients } from './service_clients';

/** Implements the ServiceClients class using HTTP requests. */
export class HttpServiceClients extends ServiceClients {

  /** Constructs all service clients. */
  constructor() {
    super();
    this.isOpen = false;
    this._serviceLocatorClient = new Beam.HttpServiceLocatorClient();
    this._administrationClient = new HttpAdministrationClient();
    this._definitionsClient = new HttpDefinitionsClient();
    this._complianceClient = new HttpComplianceClient();
    this._riskClient = new HttpRiskClient();
  }

  public get serviceLocatorClient(): Beam.HttpServiceLocatorClient {
    return this._serviceLocatorClient;
  }

  public get administrationClient(): HttpAdministrationClient {
    return this._administrationClient;
  }

  public get definitionsClient(): HttpDefinitionsClient {
    return this._definitionsClient;
  }

  public get complianceClient(): HttpComplianceClient {
    return this._complianceClient;
  }

  public get riskClient(): HttpRiskClient {
    return this._riskClient;
  }

  public async login(username: string, password: string):
      Promise<Beam.DirectoryEntry> {
    const account = await this._serviceLocatorClient.login(username, password);
    await this.open();
    return account;
  }

  public async open(): Promise<void> {
    if(this.isOpen) {
      return;
    }
    await this._administrationClient.open();
    await this._definitionsClient.open();
    await this._complianceClient.open();
    await this._riskClient.open();
    this.isOpen = true;
  }

  public async close(): Promise<void> {
    this.isOpen = false;
    await this._riskClient.close();
    await this._complianceClient.close();
    await this._definitionsClient.close();
    await this._administrationClient.close();
    await this._serviceLocatorClient.close();
  }

  private isOpen: boolean;
  private _serviceLocatorClient: Beam.HttpServiceLocatorClient;
  private _administrationClient: HttpAdministrationClient;
  private _definitionsClient: HttpDefinitionsClient;
  private _complianceClient: HttpComplianceClient;
  private _riskClient: HttpRiskClient;
}
