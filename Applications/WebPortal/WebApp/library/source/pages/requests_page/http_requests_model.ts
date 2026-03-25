import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalRequestsModel } from './local_requests_model';
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
    this.localModel = null;
  }

  public async load(): Promise<void> {
    const admin = this.serviceClients.administrationClient;
    const ownIds = await admin.loadAccountModificationRequestIds(
      this.account, 0, MAX_REQUEST_IDS);
    const managedIds = await admin.loadManagedAccountModificationRequestIds(
      this.account, 0, MAX_REQUEST_IDS);
    const allIds = mergeIds(ownIds, managedIds);
    const entries: RequestsModel.RequestEntry[] = [];
    for(const id of allIds) {
      const entry = await this.fetchEntry(id);
      entries.push(entry);
    }
    const roles = await admin.loadAccountRoles(this.account);
    this.accessRole = getAccessRole(roles);
    this.localModel =
      new LocalRequestsModel(this.account, entries, new Map());
    await this.buildGrantedChain(allIds);
  }

  public async loadRequestDirectory(submission: RequestsModel.Submission):
      Promise<RequestsModel.Response> {
    try {
      return this.localModel.loadRequestDirectory(submission);
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
    try {
      return await this.localModel.loadRequestDetail(id);
    } catch {
      const detail = await this.fetchDetail(id);
      this.localModel.addDetail(detail);
      return detail;
    }
  }

  public async approve(id: number, effectiveDate: Beam.DateTime, comment: string):
      Promise<Nexus.AccountModificationRequest.Update> {
    const message = comment.length > 0 ?
      Nexus.Message.fromPlainText(comment) : new Nexus.Message();
    const update = await this.serviceClients.administrationClient.
      approveAccountModificationRequest(id, message, effectiveDate);
    this.localModel.updateEntry(id, update);
    await this.refreshDetail(id);
    return update;
  }

  public async reject(id: number, comment: string):
      Promise<Nexus.AccountModificationRequest.Update> {
    const message = comment.length > 0 ?
      Nexus.Message.fromPlainText(comment) : new Nexus.Message();
    const update = await this.serviceClients.administrationClient.
      rejectAccountModificationRequest(id, message);
    this.localModel.updateEntry(id, update);
    await this.refreshDetail(id);
    return update;
  }


  private async fetchEntry(id: number): Promise<RequestsModel.RequestEntry> {
    const admin = this.serviceClients.administrationClient;
    const request = await admin.loadAccountModificationRequest(id);
    const status = await admin.loadAccountModificationRequestStatus(id);
    const messageIds = await admin.loadMessageIds(id);
    const firstChange = await this.loadFirstChange(request);
    const changeCount = await this.getChangeCount(request);
    return {
      id: request.id,
      category: request.type,
      state: status.status,
      updateTime: status.timestamp.toDate(),
      account: request.account,
      effectiveDate: request.effectiveDate.toDate(),
      firstChange,
      additionalChangesCount: Math.max(0, changeCount - 1),
      commentCount: messageIds.length,
      managerApproval: toManagerApproval(status)
    };
  }

  private async fetchDetail(id: number): Promise<RequestsModel.RequestDetail> {
    const admin = this.serviceClients.administrationClient;
    const request = await admin.loadAccountModificationRequest(id);
    const updates = await admin.loadAccountModificationRequestUpdates(id);
    const accountIdentity = await tryLoadIdentity(admin, request.account);
    const submitterIdentity =
      await tryLoadIdentity(admin, request.submissionAccount);
    const changes = await this.loadChanges(request);
    const comments = await this.loadActivityList(id);
    const statusEntries: RequestsModel.ActivityEntry[] = [];
    statusEntries.push({
      account: toAccountProfile(request.submissionAccount, submitterIdentity),
      activity: Nexus.AccountModificationRequest.Status.PENDING,
      timestamp: request.timestamp.toDate()
    });
    for(const update of updates) {
      if(update.status !== Nexus.AccountModificationRequest.Status.PENDING) {
        const updateIdentity = await tryLoadIdentity(admin, update.account);
        statusEntries.push({
          account: toAccountProfile(update.account, updateIdentity),
          activity: update.status,
          timestamp: update.timestamp.toDate()
        });
      }
    }
    const activityList = [...statusEntries, ...comments];
    activityList.sort(
      (a, b) => a.timestamp.getTime() - b.timestamp.getTime());
    const latestUpdate = updates.length > 0 ? updates[updates.length - 1] :
      null;
    return {
      id: request.id,
      category: request.type,
      state: latestUpdate?.status ??
        Nexus.AccountModificationRequest.Status.PENDING,
      createdTime: request.timestamp.toDate(),
      updateTime: latestUpdate?.timestamp.toDate() ??
        request.timestamp.toDate(),
      account: toAccountProfile(request.account, accountIdentity),
      requester: toAccountProfile(request.submissionAccount, submitterIdentity),
      effectiveDate: request.effectiveDate,
      changes,
      activityList,
      accessRole: this.accessRole
    };
  }

  private async refreshDetail(id: number): Promise<void> {
    try {
      const detail = await this.fetchDetail(id);
      this.localModel.addDetail(detail);
    } catch {
    }
  }

  private async loadFirstChange(request: Nexus.AccountModificationRequest):
      Promise<RequestsModel.ListChange> {
    const admin = this.serviceClients.administrationClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      const modification = await admin.loadRiskModification(request.id);
      const oldState = await this.loadOldRiskState(request);
      return toFirstRiskChange(oldState, modification.parameters,
        this.serviceClients.definitionsClient.currencyDatabase);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification = await admin.loadEntitlementModification(request.id);
      const oldState = await this.loadOldEntitlementState(request);
      return toFirstEntitlementChange(oldState, modification.entitlements,
        this.serviceClients.definitionsClient.entitlementDatabase,
        this.serviceClients.definitionsClient.currencyDatabase);
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
      const modification = await admin.loadRiskModification(request.id);
      const oldState = await this.loadOldRiskState(request);
      return countRiskChanges(oldState, modification.parameters);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification = await admin.loadEntitlementModification(request.id);
      const oldState = await this.loadOldEntitlementState(request);
      return RequestsModel.computeEntitlementChanges(oldState,
        modification.entitlements,
        this.serviceClients.definitionsClient.entitlementDatabase,
        this.serviceClients.definitionsClient.currencyDatabase).length;
    }
    return 0;
  }

  private async loadChanges(request: Nexus.AccountModificationRequest):
      Promise<RequestsModel.DetailChange[]> {
    const admin = this.serviceClients.administrationClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      const modification = await admin.loadRiskModification(request.id);
      const oldState = await this.loadOldRiskState(request);
      return toRiskChanges(oldState, modification.parameters,
        this.serviceClients.definitionsClient.currencyDatabase);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const modification = await admin.loadEntitlementModification(request.id);
      const oldState = await this.loadOldEntitlementState(request);
      return toEntitlementChanges(oldState, modification.entitlements,
        this.serviceClients.definitionsClient.entitlementDatabase,
        this.serviceClients.definitionsClient.currencyDatabase);
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
      const identity = await tryLoadIdentity(admin, message.account);
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

  private async buildGrantedChain(allIds: number[]): Promise<void> {
    const admin = this.serviceClients.administrationClient;
    const chains = new Map<string, {id: number;
        grantTimestamp: Date}[]>();
    for(const id of allIds) {
      const request = await admin.loadAccountModificationRequest(id);
      const status = await admin.loadAccountModificationRequestStatus(id);
      if(status.status === Nexus.AccountModificationRequest.Status.GRANTED) {
        const key = `${request.account.id}:${request.type}`;
        if(!chains.has(key)) {
          chains.set(key, []);
        }
        chains.get(key).push({id, grantTimestamp: status.timestamp.toDate()});
      }
    }
    for(const chain of chains.values()) {
      chain.sort(
        (a, b) => a.grantTimestamp.getTime() - b.grantTimestamp.getTime());
      for(let i = 1; i < chain.length; ++i) {
        this.previousGrantedRequest.set(chain[i].id, chain[i - 1].id);
      }
    }
  }

  private async loadOldRiskState(
      request: Nexus.AccountModificationRequest):
      Promise<Nexus.RiskParameters> {
    const admin = this.serviceClients.administrationClient;
    const status = await admin.loadAccountModificationRequestStatus(request.id);
    if(status.status !== Nexus.AccountModificationRequest.Status.GRANTED) {
      return admin.loadRiskParameters(request.account);
    }
    const previousId = this.previousGrantedRequest.get(request.id);
    if(previousId === undefined) {
      return Nexus.RiskParameters.INVALID;
    }
    const previousModification = await admin.loadRiskModification(previousId);
    return previousModification.parameters;
  }

  private async loadOldEntitlementState(
      request: Nexus.AccountModificationRequest):
      Promise<Beam.Set<Beam.DirectoryEntry>> {
    const admin = this.serviceClients.administrationClient;
    const status = await admin.loadAccountModificationRequestStatus(request.id);
    if(status.status !== Nexus.AccountModificationRequest.Status.GRANTED) {
      return admin.loadAccountEntitlements(request.account);
    }
    const previousId = this.previousGrantedRequest.get(request.id);
    if(previousId === undefined) {
      return new Beam.Set<Beam.DirectoryEntry>();
    }
    const previousModification =
      await admin.loadEntitlementModification(previousId);
    return previousModification.entitlements;
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private localModel: LocalRequestsModel;
  private accessRole: Nexus.AccountRoles.Role;
  private previousGrantedRequest: Map<number, number> = new Map();
}

const MAX_REQUEST_IDS = 1000;

async function tryLoadIdentity(
    admin: Nexus.AdministrationClient,
    account: Beam.DirectoryEntry): Promise<Nexus.AccountIdentity | undefined> {
  try {
    return await admin.loadAccountIdentity(account);
  } catch {
    return undefined;
  }
}

function mergeIds(ownIds: number[], managedIds: number[]): number[] {
  const set = new Set(ownIds);
  for(const id of managedIds) {
    set.add(id);
  }
  return [...set];
}

function getAccessRole(roles: Nexus.AccountRoles): Nexus.AccountRoles.Role {
  if(roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
    return Nexus.AccountRoles.Role.ADMINISTRATOR;
  }
  if(roles.test(Nexus.AccountRoles.Role.MANAGER)) {
    return Nexus.AccountRoles.Role.MANAGER;
  }
  return Nexus.AccountRoles.Role.TRADER;
}

function toAccountProfile(account: Beam.DirectoryEntry,
    identity?: Nexus.AccountIdentity): RequestsModel.AccountProfile {
  const first = identity?.firstName || '';
  const last = identity?.lastName || '';
  const initials = ((first[0] || '') + (last[0] || '')).toUpperCase() ||
    account.name.substring(0, 1).toUpperCase();
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

function toManagerApproval(status: Nexus.AccountModificationRequest.Update):
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
    requested: Nexus.RiskParameters,
    currencyDatabase: Nexus.CurrencyDatabase): RequestsModel.RiskControlsChange {
  if(!current.currency.equals(requested.currency)) {
    return {
      type: 'risk_controls',
      name: 'Currency',
      oldValue: currencyDatabase.fromCurrency(current.currency).code,
      newValue: currencyDatabase.fromCurrency(requested.currency).code,
      delta: {value: '', direction: RequestsModel.Direction.NONE}
    };
  }
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
      value: diff.toString(),
      direction: cmp > 0 ? RequestsModel.Direction.POSITIVE :
        cmp < 0 ? RequestsModel.Direction.NEGATIVE :
        RequestsModel.Direction.NONE
    }
  };
}

function toFirstEntitlementChange(current: Beam.Set<Beam.DirectoryEntry>,
    requested: Beam.Set<Beam.DirectoryEntry>,
    entitlementDatabase: Nexus.EntitlementDatabase,
    currencyDatabase: Nexus.CurrencyDatabase):
      RequestsModel.EntitlementsChange {
  for(const entry of requested) {
    if(!current.has(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = info.group.equals(Beam.DirectoryEntry.INVALID) ?
        entry.name : info.name;
      const isFree = info.price.equals(Nexus.Money.ZERO);
      return {
        type: 'entitlements',
        name,
        action: RequestsModel.EntitlementAction.GRANT,
        fee: info.price,
        currency: currencyDatabase.fromCurrency(info.currency),
        direction: isFree ? RequestsModel.Direction.NONE :
          RequestsModel.Direction.POSITIVE
      };
    }
  }
  for(const entry of current) {
    if(!requested.has(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = info.group.equals(Beam.DirectoryEntry.INVALID) ?
        entry.name : info.name;
      return {
        type: 'entitlements',
        name,
        action: RequestsModel.EntitlementAction.REVOKE,
        fee: info.price,
        currency: currencyDatabase.fromCurrency(info.currency),
        direction: RequestsModel.Direction.NEGATIVE
      };
    }
  }
  return {
    type: 'entitlements',
    name: 'Unknown',
    action: RequestsModel.EntitlementAction.GRANT,
    fee: Nexus.Money.ZERO,
    currency: undefined
  };
}

function countRiskChanges(current: Nexus.RiskParameters,
    requested: Nexus.RiskParameters): number {
  let count = 0;
  if(!current.currency.equals(requested.currency)) { ++count; }
  if(!current.buyingPower.equals(requested.buyingPower)) { ++count; }
  if(!current.netLoss.equals(requested.netLoss)) { ++count; }
  if(!current.transitionTime.equals(requested.transitionTime)) { ++count; }
  if(riskStateToString(current.allowedState) !==
      riskStateToString(requested.allowedState)) { ++count; }
  return count;
}

function toRiskChanges(current: Nexus.RiskParameters,
    requested: Nexus.RiskParameters,
    currencyDatabase: Nexus.CurrencyDatabase): RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  addRiskChange(changes, 'Currency',
    currencyDatabase.fromCurrency(current.currency).code,
    currencyDatabase.fromCurrency(requested.currency).code);
  addMoneyRiskChange(
    changes, 'Buying Power', current.buyingPower, requested.buyingPower);
  addMoneyRiskChange(
    changes, 'Net Loss', current.netLoss, requested.netLoss);
  addDurationRiskChange(changes, 'Transition Time',
    current.transitionTime, requested.transitionTime);
  addRiskChange(
    changes, 'Allowed State', riskStateToString(current.allowedState),
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
      value: diff.toString(),
      direction: cmp > 0 ? RequestsModel.Direction.POSITIVE :
        cmp < 0 ? RequestsModel.Direction.NEGATIVE :
        RequestsModel.Direction.NONE
    }
  });
}

function addDurationRiskChange(changes: RequestsModel.DetailChange[],
    name: string, oldValue: Beam.Duration, newValue: Beam.Duration) {
  if(oldValue.equals(newValue)) {
    return;
  }
  const diff = newValue.subtract(oldValue);
  const cmp = diff.compare(Beam.Duration.ZERO);
  changes.push({
    type: 'risk',
    name: name,
    oldValue: oldValue.toString(),
    newValue: newValue.toString(),
    delta: {
      value: diff.toString(),
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
    requested: Beam.Set<Beam.DirectoryEntry>,
    entitlementDatabase: Nexus.EntitlementDatabase,
    currencyDatabase: Nexus.CurrencyDatabase):
      RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  for(const entry of requested) {
    if(!current.has(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = info.group.equals(Beam.DirectoryEntry.INVALID) ?
        entry.name : info.name;
      changes.push({
        type: 'entitlement',
        name,
        oldStatus: RequestsModel.EntitlementStatus.REVOKED,
        newStatus: RequestsModel.EntitlementStatus.GRANTED,
        delta: toEntitlementDelta(info, currencyDatabase,
          RequestsModel.EntitlementStatus.GRANTED)
      });
    }
  }
  for(const entry of current) {
    if(!requested.has(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = info.group.equals(Beam.DirectoryEntry.INVALID) ?
        entry.name : info.name;
      changes.push({
        type: 'entitlement',
        name,
        oldStatus: RequestsModel.EntitlementStatus.GRANTED,
        newStatus: RequestsModel.EntitlementStatus.REVOKED,
        delta: toEntitlementDelta(info, currencyDatabase,
          RequestsModel.EntitlementStatus.REVOKED)
      });
    }
  }
  return changes;
}

function toEntitlementDelta(info: Nexus.EntitlementDatabase.Entry,
    currencyDatabase: Nexus.CurrencyDatabase,
    status: RequestsModel.EntitlementStatus): RequestsModel.Delta {
  if(info.price.equals(Nexus.Money.ZERO)) {
    return {value: 'FREE', direction: RequestsModel.Direction.NONE};
  }
  const currency = currencyDatabase.fromCurrency(info.currency);
  return {
    value: `${currency.sign}${info.price.toString()}`,
    direction: status === RequestsModel.EntitlementStatus.GRANTED ?
      RequestsModel.Direction.POSITIVE : RequestsModel.Direction.NEGATIVE
  };
}
