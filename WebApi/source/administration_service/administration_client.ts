import * as Beam from 'beam';
import {Message, RiskParameters} from '..';
import {AccountModificationRequest, AccountRoles, RiskModification} from '.';

/** Client used to access administration services. */
export abstract class AdministrationClient {

  /** Loads an account's roles.
   * @param account - The directory entry of the account.
   * @return The account's roles.
   */
  public abstract async loadAccountRoles(account: Beam.DirectoryEntry):
    Promise<AccountRoles>;

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
