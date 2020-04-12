import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from '..';

/** Base class for the model used by the DashboardPage. */
export abstract class DashboardModel {

  /** Returns the database of entitlements. */
  public abstract get entitlementDatabase(): Nexus.EntitlementDatabase;

  /** Returns the database of countries. */
  public abstract get countryDatabase(): Nexus.CountryDatabase;

  /** Returns the database of currencies. */
  public abstract get currencyDatabase(): Nexus.CurrencyDatabase;

  /** Returns the database of markets. */
  public abstract get marketDatabase(): Nexus.MarketDatabase;

  /** The account currently logged in. */
  public abstract get account(): Beam.DirectoryEntry;

  /** The account's roles. */
  public abstract get roles(): Nexus.AccountRoles;

  /** Makes a new account model. */
  public abstract makeAccountModel(account: Beam.DirectoryEntry): AccountModel;

  /** Loads this model. */
  public abstract async load(): Promise<void>;

  /** Logs the user out. */
  public abstract async logout(): Promise<void>;
}
