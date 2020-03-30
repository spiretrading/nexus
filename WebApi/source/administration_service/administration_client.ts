import * as Beam from 'beam';
import { Message, RiskParameters } from '..';
import { AccountIdentity } from './account_identity';
import { AccountModificationRequest } from './account_modification_request';
import { AccountRoles } from './account_roles';
import { EntitlementModification } from './entitlement_modification';
import { RiskModification } from './risk_modification';

/** Client used to access administration services. */
export abstract class AdministrationClient {

  /** Loads an account's roles.
   * @param account - The directory entry of the account.
   * @return The account's roles.
   */
  public abstract async loadAccountRoles(account: Beam.DirectoryEntry):
    Promise<AccountRoles>;

  /** Loads an account's identity.
   * @param account - The directory entry of the account.
   * @return The account's identity.
   */
  public abstract async loadIdentity(account: Beam.DirectoryEntry):
    Promise<AccountIdentity>;

  /** Loads an account's entitlements.
   * @param account - The account whose entitlements are to be loaded.
   * @return The account's entitlements.
   */
  public abstract async loadAccountEntitlements(account: Beam.DirectoryEntry):
    Promise<Beam.Set<Beam.DirectoryEntry>>;

  /** Loads an entitlement modification.
   * @param id - The id of the modification to load.
   * @return The entitlement modification with the specified id.
   */
  public abstract async loadEntitlementModification(id: number):
    Promise<EntitlementModification>;

  /** Submits a request to modify an account's entitlements.
   * @param account - The account to modify.
   * @param modification - The modification to apply.
   * @param comment - The comment to associate with the request.
   * @return An object representing the request.
   */
  public abstract async submitEntitlementModificationRequest(
    account: Beam.DirectoryEntry, modification: EntitlementModification,
    comment: Message): Promise<AccountModificationRequest>;

  /** Loads an account's risk parameters.
   * @param account - The account to load.
   * @return The account's risk parameters.
   */
  public abstract async loadRiskParameters(account: Beam.DirectoryEntry):
    Promise<RiskParameters>;

  /** Loads a risk modification.
   * @param id - The id of the modification to load.
   * @return The risk modification with the specified id.
   */
  public abstract async loadRiskModification(id: number):
    Promise<RiskModification>;

  /** Submits a request to modify an account's risk parameters.
   * @param account - The account to modify.
   * @param modification - The modification to apply.
   * @param comment - The comment to associate with the request.
   * @return An object representing the request.
   */
  public abstract async submitRiskModificationRequest(
    account: Beam.DirectoryEntry, modification: RiskModification,
    comment: Message): Promise<AccountModificationRequest>;

  /** Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract async open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract async close(): Promise<void>;
}
