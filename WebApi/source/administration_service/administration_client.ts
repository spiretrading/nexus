import * as Beam from 'beam';
import { Message, RiskParameters } from '..';
import { AccountIdentity } from './account_identity';
import { AccountModificationRequest } from './account_modification_request';
import { AccountRoles } from './account_roles';
import { EntitlementModification } from './entitlement_modification';
import { RiskModification } from './risk_modification';
import { TradingGroup } from './trading_group';

/** Client used to access administration services. */
export abstract class AdministrationClient {

  /**
   * Searches for accounts whose name matches a prefix.
   * @param prefix The prefix to match.
   * @return A list of matching tuples where the first element is a group, the
   *         second element is a matching DirectoryEntry belonging
   *         to that group, and the third element is the account's roles.
   */
  public abstract searchAccounts(prefix: string):
    Promise<[Beam.DirectoryEntry, Beam.DirectoryEntry, AccountRoles][]>;

  /**
   * Loads a TradingGroup.
   * @param directoryEntry The DirectoryEntry identifying the TradingGroup to
   *        load.
   * @return The TradingGroup identified by the directoryEntry.
   */
  public abstract loadTradingGroup(directoryEntry: Beam.DirectoryEntry):
    Promise<TradingGroup>;

  /**
   * Loads all trading groups managed by an account.
   * @param account The account whose trading groups are to be loaded.
   * @return The list of DirectoryEntry's representing the trading groups
   *         managed by the specified account.
   */
  public abstract loadManagedTradingGroups(account: Beam.DirectoryEntry):
    Promise<Beam.DirectoryEntry[]>;

  /**
   * Loads the DirectoryEntry representing an account's trading group.
   * @param account The account whose trading group is to be loaded.
   * @return The directory of the <i>account</i>'s trading group.
   */
  public abstract loadParentTradingGroup(account: Beam.DirectoryEntry):
    Promise<Beam.DirectoryEntry>;

  /**
   * Loads an account's roles.
   * @param account - The directory entry of the account.
   * @return The account's roles.
   */
  public abstract loadAccountRoles(account: Beam.DirectoryEntry):
    Promise<AccountRoles>;

  /**
   * Stores an account's roles.
   * @param account - The account to modify.
   * @param roles - The account's new roles.
   * @return The account's updated roles.
   */
  public abstract storeAccountRoles(account: Beam.DirectoryEntry,
    roles: AccountRoles): Promise<AccountRoles>;

  /**
   * Loads an account's identity.
   * @param account - The directory entry of the account.
   * @return The account's identity.
   */
  public abstract loadAccountIdentity(account: Beam.DirectoryEntry):
    Promise<AccountIdentity>;

  /**
   * Stores an account's identity.
   * @param account - The account to modify.
   * @param identity - The account's new identity.
   */
  public abstract storeAccountIdentity(account: Beam.DirectoryEntry,
    identity: AccountIdentity): Promise<void>;

  /**
   * Loads an account's entitlements.
   * @param account - The account whose entitlements are to be loaded.
   * @return The account's entitlements.
   */
  public abstract loadAccountEntitlements(account: Beam.DirectoryEntry):
    Promise<Beam.Set<Beam.DirectoryEntry>>;

  /**
   * Loads an entitlement modification.
   * @param id - The id of the modification to load.
   * @return The entitlement modification with the specified id.
   */
  public abstract loadEntitlementModification(id: number):
    Promise<EntitlementModification>;

  /**
   * Submits a request to modify an account's entitlements.
   * @param account - The account to modify.
   * @param modification - The modification to apply.
   * @param comment - The comment to associate with the request.
   * @return An object representing the request.
   */
  public abstract submitEntitlementModificationRequest(
    account: Beam.DirectoryEntry, modification: EntitlementModification,
    comment: Message): Promise<AccountModificationRequest>;

  /**
   * Loads an account's risk parameters.
   * @param account - The account to load.
   * @return The account's risk parameters.
   */
  public abstract loadRiskParameters(account: Beam.DirectoryEntry):
    Promise<RiskParameters>;

  /**
   * Loads a risk modification.
   * @param id - The id of the modification to load.
   * @return The risk modification with the specified id.
   */
  public abstract loadRiskModification(id: number):
    Promise<RiskModification>;

  /**
   * Submits a request to modify an account's risk parameters.
   * @param account - The account to modify.
   * @param modification - The modification to apply.
   * @param comment - The comment to associate with the request.
   * @return An object representing the request.
   */
  public abstract submitRiskModificationRequest(
    account: Beam.DirectoryEntry, modification: RiskModification,
    comment: Message): Promise<AccountModificationRequest>;

  /**
   * Creates a new trading group.
   * @param name The name of the group.
   * @return The new group's DirectoryEntry.
   */
  public abstract createGroup(name: string): Promise<Beam.DirectoryEntry>;

  /**
   * Creates a new account.
   * @param name The name of the account.
   * @param group The group the account belongs to.
   * @param identity The new account's identity.
   * @param roles The new account's roles.
   * @return The new account's DirectoryEntry.
   */
  public abstract createAccount(name: string, group: Beam.DirectoryEntry,
    identity: AccountIdentity, roles: AccountRoles):
    Promise<Beam.DirectoryEntry>;

  /**
   * Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
