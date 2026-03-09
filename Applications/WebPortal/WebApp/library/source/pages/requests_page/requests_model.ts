import * as Beam from 'beam';
import * as Nexus from 'nexus';

type Type = Nexus.AccountModificationRequest.Type;
type Status = Nexus.AccountModificationRequest.Status;

/** Model used by the RequestsPage. */
export abstract class RequestsModel {

  /** Loads a page of requests matching the given submission criteria.
   *  @param submission - The filter, sort, and pagination criteria.
   *  @return The response containing request entries and facet counts.
   */
  public abstract loadRequestDirectory(submission: RequestsModel.Submission):
    Promise<RequestsModel.Response>;

  /** Loads the full details for a single request.
   *  @param id - The request id.
   *  @return All data needed to render the detail page.
   */
  public abstract loadRequestDetail(id: number):
    Promise<RequestsModel.RequestDetail>;

  /** Approves a request.
   *  @param id - The request id.
   *  @param effectiveDate - The effective date for the change.
   *  @param comment - An optional comment.
   *  @return The resulting status update.
   */
  public abstract approve(id: number, effectiveDate: Beam.Date,
    comment: string): Promise<Nexus.AccountModificationRequest.Update>;

  /** Rejects a request.
   *  @param id - The request id.
   *  @param comment - An optional comment.
   *  @return The resulting status update.
   */
  public abstract reject(id: number, comment: string):
    Promise<Nexus.AccountModificationRequest.Update>;
}

export namespace RequestsModel {

  /** Enumerates the direction of a change. */
  export enum Direction {

    /** The value increased. */
    POSITIVE,

    /** The value decreased. */
    NEGATIVE,

    /** No directional change. */
    NONE
  }

  /** Enumerates the action taken on an entitlement. */
  export enum EntitlementAction {

    /** The entitlement is being granted. */
    GRANT,

    /** The entitlement is being revoked. */
    REVOKE
  }

  /** Enumerates the status of an entitlement. */
  export enum EntitlementStatus {

    /** The entitlement has been granted. */
    GRANTED,

    /** The entitlement has been revoked. */
    REVOKED
  }

  /** Enumerates the fields by which requests can be sorted. */
  export enum SortField {

    /** Sort by the last updated time. */
    LAST_UPDATED,

    /** Sort by the creation time. */
    CREATED,

    /** Sort by the account name. */
    ACCOUNT,

    /** Sort by the requester. */
    REQUESTER,

    /** Sort by the effective date. */
    EFFECTIVE_DATE
  }

  /** The scope of the displayed requests. */
  export enum Scope {
    YOU,
    GROUP
  }

  /** The currently selected request state facet. */
  export enum RequestState {
    PENDING,
    APPROVED,
    REJECTED
  }

  /** The status of the server response. */
  export enum ResponseStatus {
    IN_PROGRESS,
    READY,
    ERROR
  }

  /** A change between two values. */
  export interface Delta {

    /** The formatted delta value. */
    value: string;

    /** The direction of the change. */
    direction: Direction;
  }

  /** Profile information for an account. */
  export interface AccountProfile {

    /** The account directory entry. */
    account: Beam.DirectoryEntry;

    /** Initials for the avatar fallback. */
    initials: string;

    /** A color for the avatar background. */
    tint: string;
  }

  /** Details of a manager's approval decision. */
  export interface ManagerApproval {

    /** The account name of the approver. */
    approver: string;

    /** Whether the viewing account is the approver. */
    self: boolean;
  }

  /** A change to a risk control parameter (list view). */
  export interface RiskControlsChange {

    /** Identifies this as a risk controls change. */
    type: 'risk_controls';

    /** The name of the risk parameter. */
    name: string;

    /** The current value. */
    oldValue: string;

    /** The requested value. */
    newValue: string;

    /** The associated delta. */
    delta: Delta;
  }

  /** A change to a market data entitlement (list view). */
  export interface EntitlementsChange {

    /** Identifies this as an entitlements change. */
    type: 'entitlements';

    /** The name of the entitlement. */
    name: string;

    /** The action taken on the entitlement. */
    action: EntitlementAction;

    /** The fee associated with the entitlement. */
    fee: Nexus.Money;

    /** The currency used to display the fee. */
    currency: Nexus.CurrencyDatabase.Entry;

    /** The fee direction. If not specified, inferred from action and fee. */
    direction?: Direction;
  }

  /** A change entry for the list view. */
  export type ListChange = RiskControlsChange | EntitlementsChange;

  /** A change to a market data entitlement (detail view). */
  export interface DetailEntitlementChange {

    /** Identifies this as an entitlement change. */
    type: 'entitlement';

    /** The name of the entitlement. */
    name: string;

    /** The current status. */
    oldStatus: EntitlementStatus;

    /** The requested status. */
    newStatus: EntitlementStatus;

    /** The associated fee delta. */
    delta?: Delta;
  }

  /** A change to a risk control parameter (detail view). */
  export interface DetailRiskChange {

    /** Identifies this as a risk change. */
    type: 'risk';

    /** The name of the risk parameter. */
    name: string;

    /** The current value. */
    oldValue: string;

    /** The requested value. */
    newValue: string;

    /** The associated delta. */
    delta?: Delta;
  }

  /** A change entry for the detail view. */
  export type DetailChange = DetailEntitlementChange | DetailRiskChange;

  /** The user's filter criteria. */
  export interface Filters {
    query: string;
    categories: Set<Type>;
    startDate?: Beam.Date;
    endDate?: Beam.Date;
    sortKey: SortField;
  }

  /** The facet counts across request states. */
  export interface FacetCounts {
    pending: number;
    approved: number;
    rejected: number;
  }

  /** A single request list entry. */
  export interface RequestEntry {
    id: number;
    category: Type;
    state: Status;
    updateTime: Date;
    accountName: string;
    effectiveDate: Date;
    firstChange: ListChange;
    additionalChangesCount: number;
    commentCount: number;
    managerApproval?: ManagerApproval;
  }

  /** The response from the server. */
  export interface Response {
    status: ResponseStatus;
    facetCounts: FacetCounts;
    requestList: RequestEntry[];
  }

  /** The submission signal payload. */
  export interface Submission {
    scope: Scope;
    requestState: RequestState;
    filters: Filters;
    pageIndex: number;
  }

  /** An entry in the activity history. */
  export interface ActivityEntry {

    /** The account that performed the activity. */
    account: AccountProfile;

    /** The activity. A Status value represents a status change, and a
     *  string represents a comment. */
    activity: Status | string;

    /** The timestamp of the activity. */
    timestamp: Date;
  }

  /** All data needed to render the detail page for one request. */
  export interface RequestDetail {

    /** The request ID. */
    id: number;

    /** The request category. */
    category: Type;

    /** The current status. */
    state: Status;

    /** When the request was created. */
    createdTime: Date;

    /** When the request was last updated. */
    updateTime: Date;

    /** The account being modified. */
    account: AccountProfile;

    /** The requester. */
    requester: AccountProfile;

    /** The effective date for the change. */
    effectiveDate: Beam.Date;

    /** The list of changes. */
    changes: DetailChange[];

    /** The activity history. */
    activityList: ActivityEntry[];

    /** The current user's access role for the request. */
    accessRole: Nexus.AccountRoles.Role;
  }

  /** Computes the list of risk control changes between two RiskParameters.
   *  @param oldParameters - The current risk parameters.
   *  @param newParameters - The requested risk parameters.
   *  @param currencyDatabase - Used to look up currency signs.
   *  @return The list of changes, one per parameter that differs.
   */
  export function computeRiskChanges(oldParameters: Nexus.RiskParameters,
      newParameters: Nexus.RiskParameters,
      currencyDatabase: Nexus.CurrencyDatabase): RiskControlsChange[] {
    const changes: RiskControlsChange[] = [];
    if(!oldParameters.currency.equals(newParameters.currency)) {
      const oldCode =
        currencyDatabase.fromCurrency(oldParameters.currency).code;
      const newCode =
        currencyDatabase.fromCurrency(newParameters.currency).code;
      changes.push({
        type: 'risk_controls',
        name: 'Currency',
        oldValue: oldCode,
        newValue: newCode,
        delta: {value: newCode, direction: Direction.NONE}
      });
    }
    if(!oldParameters.buyingPower.equals(newParameters.buyingPower)) {
      changes.push(makeMoneyChange('Buying Power',
        oldParameters.buyingPower, newParameters.buyingPower,
        currencyDatabase.fromCurrency(newParameters.currency).sign));
    }
    if(!oldParameters.netLoss.equals(newParameters.netLoss)) {
      changes.push(makeMoneyChange('Net Loss',
        oldParameters.netLoss, newParameters.netLoss,
        currencyDatabase.fromCurrency(newParameters.currency).sign));
    }
    if(!oldParameters.transitionTime.equals(newParameters.transitionTime)) {
      changes.push(makeDurationChange('Transition Time',
        oldParameters.transitionTime, newParameters.transitionTime));
    }
    return changes;
  }

  /** Computes the list of entitlement changes between two entitlement sets.
   *  @param oldEntitlements - The current set of entitlements.
   *  @param newEntitlements - The requested set of entitlements.
   *  @param entitlementDatabase - Used to look up entitlement details.
   *  @param currencyDatabase - Used to look up currency signs.
   *  @return The list of changes, one per entitlement that was granted or
   *          revoked.
   */
  export function computeEntitlementChanges(
      oldEntitlements: Beam.Set<Beam.DirectoryEntry>,
      newEntitlements: Beam.Set<Beam.DirectoryEntry>,
      entitlementDatabase: Nexus.EntitlementDatabase,
      currencyDatabase: Nexus.CurrencyDatabase): EntitlementsChange[] {
    const changes: EntitlementsChange[] = [];
    for(const entry of newEntitlements) {
      if(!oldEntitlements.test(entry)) {
        const info = entitlementDatabase.fromGroup(entry);
        changes.push({
          type: 'entitlements',
          name: info.name,
          action: EntitlementAction.GRANT,
          fee: info.price,
          currency: currencyDatabase.fromCurrency(info.currency)
        });
      }
    }
    for(const entry of oldEntitlements) {
      if(!newEntitlements.test(entry)) {
        const info = entitlementDatabase.fromGroup(entry);
        changes.push({
          type: 'entitlements',
          name: info.name,
          action: EntitlementAction.REVOKE,
          fee: info.price,
          currency: currencyDatabase.fromCurrency(info.currency)
        });
      }
    }
    return changes;
  }
}

function makeMoneyChange(name: string, oldValue: Nexus.Money,
    newValue: Nexus.Money, sign: string): RequestsModel.RiskControlsChange {
  const diff = newValue.subtract(oldValue);
  const direction = diff.compare(Nexus.Money.ZERO) > 0 ?
    RequestsModel.Direction.POSITIVE :
    diff.compare(Nexus.Money.ZERO) < 0 ?
    RequestsModel.Direction.NEGATIVE : RequestsModel.Direction.NONE;
  return {
    type: 'risk_controls',
    name,
    oldValue: `${sign}${oldValue}`,
    newValue: `${sign}${newValue}`,
    delta: {
      value: `${sign}${diff.compare(Nexus.Money.ZERO) < 0 ?
        Nexus.Money.ZERO.subtract(diff) : diff}`,
      direction
    }
  };
}

function formatDuration(duration: Beam.Duration): string {
  const parts = duration.split();
  const segments: string[] = [];
  if(parts.hours > 0) {
    segments.push(`${parts.hours} ${parts.hours === 1 ? 'hour' : 'hours'}`);
  }
  if(parts.minutes > 0) {
    segments.push(
      `${parts.minutes} ${parts.minutes === 1 ? 'minute' : 'minutes'}`);
  }
  if(parts.seconds > 0 || segments.length === 0) {
    segments.push(
      `${parts.seconds} ${parts.seconds === 1 ? 'second' : 'seconds'}`);
  }
  return segments.join(' ');
}

function makeDurationChange(name: string, oldValue: Beam.Duration,
    newValue: Beam.Duration): RequestsModel.RiskControlsChange {
  const cmp = newValue.compare(oldValue);
  const direction = cmp > 0 ? RequestsModel.Direction.POSITIVE :
    cmp < 0 ? RequestsModel.Direction.NEGATIVE :
    RequestsModel.Direction.NONE;
  const diff = cmp >= 0 ?
    newValue.subtract(oldValue) : oldValue.subtract(newValue);
  return {
    type: 'risk_controls',
    name,
    oldValue: formatDuration(oldValue),
    newValue: formatDuration(newValue),
    delta: {value: formatDuration(diff), direction}
  };
}
