import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { RequestsModel } from './requests_model';

/** Implements the RequestsModel using HTTP requests. */
export class HttpRequestsModel extends RequestsModel {

  /**
   * Constructs an HttpRequestsModel.
   * @param account - The logged-in account.
   * @param serviceClients - The ServiceClients used to query.
   */
  constructor(
      account: Beam.DirectoryEntry, serviceClients: Nexus.ServiceClients) {
    super();
    this.account = account;
    this.serviceClients = serviceClients;
  }

  public async loadRequestDirectory(
      submission: RequestsModel.Submission):
      Promise<RequestsModel.Response> {
    try {
      const ids = await this.loadIds(submission);
      const requests = await this.loadRequestEntries(ids);
      const filtered = this.filterEntries(requests, submission);
      const sorted = this.sortEntries(filtered, submission.filters.sortKey);
      const facetCounts = this.computeFacetCounts(requests, submission);
      const stateFiltered = sorted.filter(
        entry => matchesState(entry.state, submission.requestState));
      return {
        status: RequestsModel.ResponseStatus.READY,
        facetCounts: facetCounts,
        requestList: stateFiltered
      };
    } catch {
      return {
        status: RequestsModel.ResponseStatus.ERROR,
        facetCounts: {pending: 0, approved: 0, rejected: 0},
        requestList: []
      };
    }
  }

  public async loadRequestDetail(id: number):
      Promise<RequestsModel.RequestDetail> {
    const admin = this.serviceClients.administrationClient;
    const request = await admin.loadAccountModificationRequest(id);
    const status = await admin.loadAccountModificationRequestStatus(id);
    const roles = await admin.loadAccountRoles(this.account);
    const accountIdentity =
      await admin.loadAccountIdentity(request.account);
    const submitterIdentity =
      await admin.loadAccountIdentity(request.submissionAccount);
    const changes = await this.loadChanges(request);
    const activityList = await this.loadActivityList(id);
    const accessRole = getAccessRole(roles);
    return {
      id: request.id,
      category: request.type,
      state: status.status,
      createdTime: request.timestamp.toDate(),
      updateTime: status.timestamp.toDate(),
      account: toAccountProfile(request.account, accountIdentity),
      requester: toAccountProfile(
        request.submissionAccount, submitterIdentity),
      effectiveDate: Beam.Date.fromDate(status.timestamp.toDate()),
      changes: changes,
      activityList: activityList,
      accessRole: accessRole
    };
  }

  public async approve(id: number, effectiveDate: Beam.Date,
      comment: string): Promise<Nexus.AccountModificationRequest.Update> {
    const message = comment.length > 0 ?
      Nexus.Message.fromPlainText(comment) : new Nexus.Message();
    return this.serviceClients.administrationClient.
      approveAccountModificationRequest(id, message);
  }

  public async reject(id: number, comment: string):
      Promise<Nexus.AccountModificationRequest.Update> {
    const message = comment.length > 0 ?
      Nexus.Message.fromPlainText(comment) : new Nexus.Message();
    return this.serviceClients.administrationClient.
      rejectAccountModificationRequest(id, message);
  }

  private async loadIds(submission: RequestsModel.Submission):
      Promise<number[]> {
    const admin = this.serviceClients.administrationClient;
    if(submission.scope === RequestsModel.Scope.GROUP) {
      return admin.loadManagedAccountModificationRequestIds(
        this.account, -1, MAX_REQUEST_IDS);
    }
    return admin.loadAccountModificationRequestIds(
      this.account, -1, MAX_REQUEST_IDS);
  }

  private async loadRequestEntries(ids: number[]):
      Promise<RequestsModel.RequestEntry[]> {
    const admin = this.serviceClients.administrationClient;
    const entries: RequestsModel.RequestEntry[] = [];
    for(const id of ids) {
      const request = await admin.loadAccountModificationRequest(id);
      const status =
        await admin.loadAccountModificationRequestStatus(id);
      const messageIds = await admin.loadMessageIds(id);
      const firstChange = await this.loadFirstChange(request);
      const changeCount = await this.getChangeCount(request);
      entries.push({
        id: request.id,
        category: request.type,
        state: status.status,
        updateTime: status.timestamp.toDate(),
        account: request.account,
        effectiveDate: status.timestamp.toDate(),
        firstChange: firstChange,
        additionalChangesCount: Math.max(0, changeCount - 1),
        commentCount: messageIds.length,
        managerApproval: toManagerApproval(status)
      });
    }
    return entries;
  }

  private async loadFirstChange(request: Nexus.AccountModificationRequest):
      Promise<RequestsModel.ListChange> {
    const admin = this.serviceClients.administrationClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      const modification = await admin.loadRiskModification(request.id);
      const current = await admin.loadRiskParameters(request.account);
      return toFirstRiskChange(current, modification.parameters);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification =
        await admin.loadEntitlementModification(request.id);
      const current = await admin.loadAccountEntitlements(request.account);
      return toFirstEntitlementChange(current, modification.entitlements);
    }
    return {
      type: 'risk_controls',
      name: 'Unknown',
      oldValue: '',
      newValue: '',
      delta: {value: '', direction: RequestsModel.Direction.NONE}
    };
  }

  private async getChangeCount(
      request: Nexus.AccountModificationRequest): Promise<number> {
    const admin = this.serviceClients.administrationClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      return RISK_PARAMETER_COUNT;
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification =
        await admin.loadEntitlementModification(request.id);
      return modification.entitlements.size;
    }
    return 0;
  }

  private async loadChanges(request: Nexus.AccountModificationRequest):
      Promise<RequestsModel.DetailChange[]> {
    const admin = this.serviceClients.administrationClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      const modification = await admin.loadRiskModification(request.id);
      const current = await admin.loadRiskParameters(request.account);
      return toRiskChanges(current, modification.parameters);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification =
        await admin.loadEntitlementModification(request.id);
      const current = await admin.loadAccountEntitlements(request.account);
      return toEntitlementChanges(current, modification.entitlements);
    }
    return [];
  }

  private async loadActivityList(requestId: number):
      Promise<RequestsModel.ActivityEntry[]> {
    const admin = this.serviceClients.administrationClient;
    const messageIds = await admin.loadMessageIds(requestId);
    const activities: RequestsModel.ActivityEntry[] = [];
    for(const messageId of messageIds) {
      const message = await admin.loadMessage(messageId);
      const identity = await admin.loadAccountIdentity(message.account);
      const plainText = message.bodies.find(
        body => body.contentType === Nexus.Message.PLAIN_TEXT);
      activities.push({
        account: toAccountProfile(message.account, identity),
        activity: plainText?.message ?? '',
        timestamp: message.timestamp.toDate()
      });
    }
    return activities;
  }

  private filterEntries(entries: RequestsModel.RequestEntry[],
      submission: RequestsModel.Submission):
      RequestsModel.RequestEntry[] {
    let result = entries;
    const filters = submission.filters;
    if(filters.categories.size > 0) {
      result = result.filter(e => filters.categories.has(e.category));
    }
    if(filters.query.length > 0) {
      const query = filters.query.toLowerCase();
      result = result.filter(e =>
        e.account.name.toLowerCase().includes(query) ||
        String(e.id).includes(query));
    }
    if(filters.startDate) {
      result = result.filter(e =>
        filters.startDate.compare(Beam.Date.fromDate(e.updateTime)) <= 0);
    }
    if(filters.endDate) {
      result = result.filter(e =>
        filters.endDate.compare(Beam.Date.fromDate(e.updateTime)) >= 0);
    }
    return result;
  }

  private sortEntries(entries: RequestsModel.RequestEntry[],
      sortKey: RequestsModel.SortField):
      RequestsModel.RequestEntry[] {
    const sorted = entries.slice();
    switch(sortKey) {
      case RequestsModel.SortField.LAST_UPDATED:
        sorted.sort((a, b) =>
          b.updateTime.getTime() - a.updateTime.getTime());
        break;
      case RequestsModel.SortField.CREATED:
        sorted.sort((a, b) =>
          b.effectiveDate.getTime() - a.effectiveDate.getTime());
        break;
      case RequestsModel.SortField.ACCOUNT:
        sorted.sort((a, b) =>
          a.account.name.localeCompare(b.account.name));
        break;
      case RequestsModel.SortField.EFFECTIVE_DATE:
        sorted.sort((a, b) =>
          b.effectiveDate.getTime() - a.effectiveDate.getTime());
        break;
      default:
        sorted.sort((a, b) =>
          b.updateTime.getTime() - a.updateTime.getTime());
        break;
    }
    return sorted;
  }

  private computeFacetCounts(entries: RequestsModel.RequestEntry[],
      submission: RequestsModel.Submission):
      RequestsModel.FacetCounts {
    const filtered = this.filterEntries(entries, submission);
    let pending = 0;
    let approved = 0;
    let rejected = 0;
    for(const entry of filtered) {
      if(matchesState(entry.state,
          RequestsModel.RequestState.PENDING)) {
        ++pending;
      }
      if(matchesState(entry.state,
          RequestsModel.RequestState.APPROVED)) {
        ++approved;
      }
      if(matchesState(entry.state,
          RequestsModel.RequestState.REJECTED)) {
        ++rejected;
      }
    }
    return {pending, approved, rejected};
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
}

const MAX_REQUEST_IDS = 1000;
const RISK_PARAMETER_COUNT = 4;

function matchesState(status: Nexus.AccountModificationRequest.Status,
    requestState: RequestsModel.RequestState): boolean {
  switch(requestState) {
    case RequestsModel.RequestState.PENDING:
      return status === Nexus.AccountModificationRequest.Status.PENDING ||
        status === Nexus.AccountModificationRequest.Status.REVIEWED ||
        status === Nexus.AccountModificationRequest.Status.SCHEDULED;
    case RequestsModel.RequestState.APPROVED:
      return status === Nexus.AccountModificationRequest.Status.GRANTED;
    case RequestsModel.RequestState.REJECTED:
      return status === Nexus.AccountModificationRequest.Status.REJECTED;
  }
}

function getAccessRole(roles: Nexus.AccountRoles):
    Nexus.AccountRoles.Role {
  if(roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
    return Nexus.AccountRoles.Role.ADMINISTRATOR;
  }
  if(roles.test(Nexus.AccountRoles.Role.MANAGER)) {
    return Nexus.AccountRoles.Role.MANAGER;
  }
  return Nexus.AccountRoles.Role.TRADER;
}

function toAccountProfile(account: Beam.DirectoryEntry,
    identity: Nexus.AccountIdentity): RequestsModel.AccountProfile {
  const first = identity.firstName || '';
  const last = identity.lastName || '';
  const initials = ((first[0] || '') + (last[0] || '')).toUpperCase() ||
    account.name.substring(0, 2).toUpperCase();
  const tint = hashToColor(account.id);
  return {account, initials, tint};
}

function hashToColor(id: number): string {
  const TINTS = [
    '#4B23A0', '#7B61FF', '#2196F3', '#00BCD4',
    '#4CAF50', '#FF9800', '#F44336', '#E91E63'
  ];
  return TINTS[Math.abs(id) % TINTS.length];
}

function toManagerApproval(
    status: Nexus.AccountModificationRequest.Update):
    RequestsModel.ManagerApproval | undefined {
  if(status.status === Nexus.AccountModificationRequest.Status.REVIEWED ||
      status.status === Nexus.AccountModificationRequest.Status.SCHEDULED) {
    return {
      approver: status.account.name,
      self: false
    };
  }
  return undefined;
}

function toFirstRiskChange(current: Nexus.RiskParameters,
    requested: Nexus.RiskParameters): RequestsModel.RiskControlsChange {
  const oldBp = current.buyingPower.toString();
  const newBp = requested.buyingPower.toString();
  const diff = requested.buyingPower.subtract(current.buyingPower);
  const cmp = diff.compare(Nexus.Money.ZERO);
  return {
    type: 'risk_controls',
    name: 'Buying Power',
    oldValue: oldBp,
    newValue: newBp,
    delta: {
      value: (cmp >= 0 ? '+' : '') + diff.toString(),
      direction: cmp > 0 ? RequestsModel.Direction.POSITIVE :
        cmp < 0 ? RequestsModel.Direction.NEGATIVE :
        RequestsModel.Direction.NONE
    }
  };
}

function toFirstEntitlementChange(
    current: Beam.Set<Beam.DirectoryEntry>,
    requested: Beam.Set<Beam.DirectoryEntry>):
    RequestsModel.EntitlementsChange {
  for(const entry of requested) {
    const isNew = !current.has(entry);
    return {
      type: 'entitlements',
      name: entry.name,
      action: isNew ? RequestsModel.EntitlementAction.GRANT :
        RequestsModel.EntitlementAction.REVOKE,
      fee: Nexus.Money.ZERO,
      currency: undefined,
      direction: isNew ? RequestsModel.Direction.POSITIVE :
        RequestsModel.Direction.NEGATIVE
    };
  }
  return {
    type: 'entitlements',
    name: 'Unknown',
    action: RequestsModel.EntitlementAction.GRANT,
    fee: Nexus.Money.ZERO,
    currency: undefined
  };
}

function toRiskChanges(current: Nexus.RiskParameters,
    requested: Nexus.RiskParameters): RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  addMoneyRiskChange(changes, 'Buying Power',
    current.buyingPower, requested.buyingPower);
  addMoneyRiskChange(changes, 'Net Loss',
    current.netLoss, requested.netLoss);
  addRiskChange(changes, 'Transition Time',
    current.transitionTime.toString(), requested.transitionTime.toString());
  addRiskChange(changes, 'Allowed State',
    riskStateToString(current.allowedState),
    riskStateToString(requested.allowedState));
  return changes;
}

function addMoneyRiskChange(changes: RequestsModel.DetailChange[],
    name: string, oldValue: Nexus.Money, newValue: Nexus.Money) {
  if(oldValue.equals(newValue)) {
    return;
  }
  const diff = newValue.subtract(oldValue);
  const cmp = diff.compare(Nexus.Money.ZERO);
  changes.push({
    type: 'risk',
    name: name,
    oldValue: oldValue.toString(),
    newValue: newValue.toString(),
    delta: {
      value: (cmp >= 0 ? '+' : '') + diff.toString(),
      direction: cmp > 0 ? RequestsModel.Direction.POSITIVE :
        cmp < 0 ? RequestsModel.Direction.NEGATIVE :
        RequestsModel.Direction.NONE
    }
  });
}

function addRiskChange(changes: RequestsModel.DetailChange[],
    name: string, oldValue: string, newValue: string) {
  if(oldValue === newValue) {
    return;
  }
  changes.push({
    type: 'risk',
    name: name,
    oldValue: oldValue,
    newValue: newValue
  });
}

function riskStateToString(state: Nexus.RiskState): string {
  switch(state.type) {
    case Nexus.RiskState.Type.ACTIVE:
      return 'Active';
    case Nexus.RiskState.Type.CLOSED_ORDERS:
      return 'Close Only';
    case Nexus.RiskState.Type.DISABLED:
      return 'Disabled';
    default:
      return 'None';
  }
}

function toEntitlementChanges(current: Beam.Set<Beam.DirectoryEntry>,
    requested: Beam.Set<Beam.DirectoryEntry>):
    RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  for(const entry of requested) {
    const isNew = !current.has(entry);
    changes.push({
      type: 'entitlement',
      name: entry.name,
      oldStatus: isNew ? RequestsModel.EntitlementStatus.REVOKED :
        RequestsModel.EntitlementStatus.GRANTED,
      newStatus: isNew ? RequestsModel.EntitlementStatus.GRANTED :
        RequestsModel.EntitlementStatus.REVOKED
    });
  }
  return changes;
}
