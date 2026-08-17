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
    this.anchors = [null];
    this.anchorKey = '';
    this.generation = 0;
    this.countsKey = null;
    this.counts = null;
  }

  public async load(): Promise<void> {
    const admin = this.serviceClients.administrationClient;
    const roles = await admin.loadAccountRoles(this.account);
    this.accessRole = getAccessRole(roles);
    this.tradingGroupsRoot = await admin.loadTradingGroupsRootEntry();
    this.localModel = new LocalRequestsModel(this.account, [], new Map());
  }

  public async loadRequestDirectory(submission: RequestsModel.Submission):
      Promise<RequestsModel.Response> {
    try {
      const admin = this.serviceClients.administrationClient;
      this.resetAnchors(submission);
      const query = this.makeQuery(submission);
      const generation = ++this.generation;
      const [summaries, counts] = await Promise.all([
        admin.loadAccountModificationRequestSummaries(query),
        this.loadCounts(makeCountsKey(submission), query)
      ]);
      if(summaries.length > 0 && generation === this.generation) {
        this.anchors[submission.pageIndex + 1] =
          makeAnchor(summaries[0], query.sortField);
      }
      const requestList =
        summaries.slice().reverse().map(summary => this.toEntry(summary));
      return {
        status: RequestsModel.ResponseStatus.READY,
        facetCounts: {
          pending: counts.pending,
          approved: counts.granted,
          rejected: counts.rejected
        },
        totalCount: facetCount(counts, submission.requestState),
        requestList
      };
    } catch {
      return {
        status: RequestsModel.ResponseStatus.ERROR,
        facetCounts: {pending: 0, approved: 0, rejected: 0},
        totalCount: 0,
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

  public async approve(id: number, effectiveDate: Beam.DateTime,
      comment: string): Promise<Nexus.AccountModificationRequest.Update> {
    const message = toMessage(comment);
    const update = await this.serviceClients.administrationClient.
      approveAccountModificationRequest(id, message, effectiveDate);
    this.invalidateCounts();
    await this.refreshDetail(id);
    return update;
  }

  public async reject(id: number, comment: string):
      Promise<Nexus.AccountModificationRequest.Update> {
    const message = toMessage(comment);
    const update = await this.serviceClients.administrationClient.
      rejectAccountModificationRequest(id, message);
    this.invalidateCounts();
    await this.refreshDetail(id);
    return update;
  }

  private async loadCounts(
      key: string, query: Nexus.AccountModificationRequestQuery):
        Promise<Nexus.AccountModificationRequestCounts> {
    if(key === this.countsKey) {
      return this.counts;
    }
    const counts = await this.serviceClients.administrationClient.
      loadAccountModificationRequestCounts(query);
    this.countsKey = key;
    this.counts = counts;
    return counts;
  }

  private invalidateCounts(): void {
    this.countsKey = null;
    this.counts = null;
  }

  private resetAnchors(submission: RequestsModel.Submission): void {
    const key = makeAnchorKey(submission);
    if(key !== this.anchorKey) {
      this.anchorKey = key;
      this.anchors = [null];
    }
  }

  private makeQuery(submission: RequestsModel.Submission):
      Nexus.AccountModificationRequestQuery {
    const isGroup = submission.scope === RequestsModel.Scope.GROUP;
    const index = (() => {
      if(isGroup) {
        return this.tradingGroupsRoot;
      }
      return this.account;
    })();
    const query = new Nexus.AccountModificationRequestQuery(
      index, Beam.SnapshotLimit.fromTail(RequestsModel.PAGE_SIZE));
    const anchor = this.anchors[submission.pageIndex];
    if(anchor === undefined) {
      query.offset = submission.pageIndex * RequestsModel.PAGE_SIZE;
    } else {
      query.anchor = anchor;
    }
    query.categories = [...submission.filters.categories];
    query.statuses = toStatuses(submission.requestState);
    query.search = submission.filters.query;
    query.sortField = toSortField(submission.filters.sortKey);
    if(submission.filters.startDate) {
      query.startDate = new Beam.DateTime(submission.filters.startDate);
    }
    if(submission.filters.endDate) {
      query.endDate = new Beam.DateTime(submission.filters.endDate, END_OF_DAY);
    }
    if(isGroup) {
      query.excludedAccount = this.account;
    }
    return query;
  }

  private toEntry(summary: Nexus.AccountModificationRequestSummary):
      RequestsModel.RequestEntry {
    const request = summary.request;
    const status = summary.status;
    const definitions = this.serviceClients.definitionsClient;
    const state = (() => {
      if(status.status === Nexus.AccountModificationRequest.Status.NONE) {
        return Nexus.AccountModificationRequest.Status.PENDING;
      }
      return status.status;
    })();
    const changes = (() => {
      if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
        const previous = summary.previousState as Nexus.RiskModification;
        const requested = summary.modification as Nexus.RiskModification;
        if(!previous || !requested) {
          return {first: unknownChange(), count: 0};
        }
        return {
          first: toFirstRiskChange(previous.parameters, requested.parameters,
            definitions.currencyDatabase),
          count: countRiskChanges(previous.parameters, requested.parameters)
        };
      }
      if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
        const previous = summary.previousState as Nexus.EntitlementModification;
        const requested = summary.modification as Nexus.EntitlementModification;
        if(!previous || !requested) {
          return {first: unknownChange(), count: 0};
        }
        return {
          first: toFirstEntitlementChange(previous.entitlements,
            requested.entitlements, definitions.entitlementDatabase,
            definitions.currencyDatabase),
          count: RequestsModel.computeEntitlementChanges(previous.entitlements,
            requested.entitlements, definitions.entitlementDatabase,
            definitions.currencyDatabase).length
        };
      }
      return {first: unknownChange(), count: 0};
    })();
    return {
      id: request.id,
      category: request.type,
      state,
      updateTime: updateTime(summary).toDate(),
      account: request.account,
      effectiveDate: request.effectiveDate.toDate(),
      firstChange: changes.first,
      additionalChangesCount: Math.max(0, changes.count - 1),
      commentCount: summary.commentCount,
      managerApproval: toManagerApproval(status)
    };
  }

  private async fetchDetail(id: number): Promise<RequestsModel.RequestDetail> {
    const admin = this.serviceClients.administrationClient;
    const request = await admin.loadAccountModificationRequest(id);
    const summary = await this.loadSummary(request);
    const updates = await admin.loadAccountModificationRequestUpdates(id);
    const accountIdentity = await tryLoadIdentity(admin, request.account);
    const submitterIdentity =
      await tryLoadIdentity(admin, request.submissionAccount);
    const changes = this.loadChanges(request, summary);
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
    activityList.sort((a, b) => a.timestamp.getTime() - b.timestamp.getTime());
    const latestUpdate = updates[updates.length - 1] ?? null;
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
      effectiveDate: request.effectiveDate.date,
      changes,
      activityList,
      accessRole: this.accessRole
    };
  }

  private async loadSummary(request: Nexus.AccountModificationRequest):
      Promise<Nexus.AccountModificationRequestSummary> {
    const query = new Nexus.AccountModificationRequestQuery(
      request.account, Beam.SnapshotLimit.fromTail(1));
    query.anchor = new Nexus.AccountModificationRequestAnchor(
      request.id + 1, request.timestamp, '');
    const summaries = await this.serviceClients.administrationClient.
      loadAccountModificationRequestSummaries(query);
    return summaries.find(summary => summary.request.id === request.id);
  }

  private async refreshDetail(id: number): Promise<void> {
    try {
      this.localModel.addDetail(await this.fetchDetail(id));
    } catch {
      this.localModel.removeDetail(id);
    }
  }

  private loadChanges(request: Nexus.AccountModificationRequest,
      summary: Nexus.AccountModificationRequestSummary):
        RequestsModel.DetailChange[] {
    if(!summary || !summary.previousState || !summary.modification) {
      return [];
    }
    const definitions = this.serviceClients.definitionsClient;
    if(request.type === Nexus.AccountModificationRequest.Type.RISK) {
      const previous = summary.previousState as Nexus.RiskModification;
      const requested = summary.modification as Nexus.RiskModification;
      return toRiskChanges(previous.parameters, requested.parameters,
        definitions.currencyDatabase);
    }
    if(request.type === Nexus.AccountModificationRequest.Type.ENTITLEMENTS) {
      const previous = summary.previousState as Nexus.EntitlementModification;
      const requested = summary.modification as Nexus.EntitlementModification;
      return toEntitlementChanges(previous.entitlements, requested.entitlements,
        definitions.entitlementDatabase, definitions.currencyDatabase);
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

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private localModel: LocalRequestsModel;
  private accessRole: Nexus.AccountRoles.Role;
  private tradingGroupsRoot: Beam.DirectoryEntry;
  private anchors: Nexus.AccountModificationRequestAnchor[];
  private anchorKey: string;
  private generation: number;
  private countsKey: string;
  private counts: Nexus.AccountModificationRequestCounts;
}

const END_OF_DAY = new Beam.Duration(24 * Beam.Duration.MINUTES_PER_HOUR *
  Beam.Duration.SECONDS_PER_MINUTE * Beam.Duration.TICKS_PER_SECOND - 1);

function toMessage(comment: string): Nexus.Message {
  if(comment.length > 0) {
    return Nexus.Message.fromPlainText(comment);
  }
  return new Nexus.Message();
}

function entitlementName(
    entry: Beam.DirectoryEntry, info: Nexus.EntitlementDatabase.Entry): string {
  if(info.group.equals(Beam.DirectoryEntry.INVALID)) {
    return entry.name;
  }
  return info.name;
}

function toSortField(sortKey: RequestsModel.SortField):
    Nexus.AccountModificationRequestQuery.SortField {
  if(sortKey === RequestsModel.SortField.LAST_UPDATED) {
    return Nexus.AccountModificationRequestQuery.SortField.LAST_UPDATED;
  } else if(sortKey === RequestsModel.SortField.EFFECTIVE_DATE) {
    return Nexus.AccountModificationRequestQuery.SortField.EFFECTIVE_DATE;
  } else if(sortKey === RequestsModel.SortField.ACCOUNT) {
    return Nexus.AccountModificationRequestQuery.SortField.ACCOUNT;
  } else if(sortKey === RequestsModel.SortField.REQUESTER) {
    return Nexus.AccountModificationRequestQuery.SortField.REQUESTER;
  }
  return Nexus.AccountModificationRequestQuery.SortField.CREATED;
}

function toStatuses(state: RequestsModel.RequestState):
    Nexus.AccountModificationRequest.Status[] {
  if(state === RequestsModel.RequestState.APPROVED) {
    return [Nexus.AccountModificationRequest.Status.GRANTED];
  }
  if(state === RequestsModel.RequestState.REJECTED) {
    return [Nexus.AccountModificationRequest.Status.REJECTED];
  }
  return [
    Nexus.AccountModificationRequest.Status.NONE,
    Nexus.AccountModificationRequest.Status.PENDING,
    Nexus.AccountModificationRequest.Status.REVIEWED,
    Nexus.AccountModificationRequest.Status.SCHEDULED
  ];
}

function updateTime(summary: Nexus.AccountModificationRequestSummary):
    Beam.DateTime {
  if(summary.status.status ===
      Nexus.AccountModificationRequest.Status.NONE) {
    return summary.request.timestamp;
  }
  return summary.status.timestamp;
}

function makeAnchor(summary: Nexus.AccountModificationRequestSummary,
    field: Nexus.AccountModificationRequestQuery.SortField):
      Nexus.AccountModificationRequestAnchor {
  const request = summary.request;
  const date = (() => {
    if(field === Nexus.AccountModificationRequestQuery.SortField.LAST_UPDATED) {
      return updateTime(summary);
    } else if(field ===
        Nexus.AccountModificationRequestQuery.SortField.EFFECTIVE_DATE) {
      return request.effectiveDate;
    }
    return request.timestamp;
  })();
  const name = (() => {
    if(field === Nexus.AccountModificationRequestQuery.SortField.ACCOUNT) {
      return request.account.name;
    } else if(field ===
        Nexus.AccountModificationRequestQuery.SortField.REQUESTER) {
      return request.submissionAccount.name;
    }
    return '';
  })();
  return new Nexus.AccountModificationRequestAnchor(request.id, date, name);
}

function toCategoryKey(
    categories: Set<Nexus.AccountModificationRequest.Type>): string {
  return [...categories].sort((left, right) => left - right).join(',');
}

function makeAnchorKey(submission: RequestsModel.Submission): string {
  return [submission.scope, submission.requestState,
    submission.filters.query, toCategoryKey(submission.filters.categories),
    submission.filters.sortKey, submission.filters.startDate?.toJson() ?? '',
    submission.filters.endDate?.toJson() ?? ''].join('|');
}

function makeCountsKey(submission: RequestsModel.Submission): string {
  return [submission.scope, submission.filters.query,
    toCategoryKey(submission.filters.categories),
    submission.filters.startDate?.toJson() ?? '',
    submission.filters.endDate?.toJson() ?? ''].join('|');
}

function facetCount(counts: Nexus.AccountModificationRequestCounts,
    state: RequestsModel.RequestState): number {
  if(state === RequestsModel.RequestState.APPROVED) {
    return counts.granted;
  }
  if(state === RequestsModel.RequestState.REJECTED) {
    return counts.rejected;
  }
  return counts.pending;
}

function unknownChange(): RequestsModel.ListChange {
  return {
    type: 'risk_controls',
    name: 'Unknown',
    oldValue: '',
    newValue: '',
    delta: {value: '', direction: RequestsModel.Direction.NONE}
  };
}

async function tryLoadIdentity(admin: Nexus.AdministrationClient,
    account: Beam.DirectoryEntry): Promise<Nexus.AccountIdentity | undefined> {
  try {
    return await admin.loadAccountIdentity(account);
  } catch {
    return undefined;
  }
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
      oldValue: RequestsModel.currencyName(current.currency, currencyDatabase),
      newValue: RequestsModel.currencyName(requested.currency, currencyDatabase),
      delta: {value: '', direction: RequestsModel.Direction.NONE}
    };
  }
  if(!current.buyingPower.equals(requested.buyingPower)) {
    return makeMoneyRiskChange('Buying Power',
      current.buyingPower, requested.buyingPower);
  }
  if(!current.netLoss.equals(requested.netLoss)) {
    return makeMoneyRiskChange('Net Loss',
      current.netLoss, requested.netLoss);
  }
  if(!current.transitionTime.equals(requested.transitionTime)) {
    const diff = requested.transitionTime.subtract(current.transitionTime);
    const cmp = diff.compare(Beam.Duration.ZERO);
    const direction = toDirection(cmp);
    return {
      type: 'risk_controls',
      name: 'Transition Time',
      oldValue: current.transitionTime.toString(),
      newValue: requested.transitionTime.toString(),
      delta: {value: diff.toString(), direction}
    };
  }
  const oldState = RequestsModel.riskStateToString(current.allowedState);
  const newState = RequestsModel.riskStateToString(requested.allowedState);
  return {
    type: 'risk_controls',
    name: 'Allowed State',
    oldValue: oldState,
    newValue: newState,
    delta: {value: '', direction: RequestsModel.Direction.NONE}
  };
}

function toDirection(comparison: number): RequestsModel.Direction {
  if(comparison > 0) {
    return RequestsModel.Direction.POSITIVE;
  }
  if(comparison < 0) {
    return RequestsModel.Direction.NEGATIVE;
  }
  return RequestsModel.Direction.NONE;
}

function makeMoneyRiskChange(name: string, oldValue: Nexus.Money,
    newValue: Nexus.Money): RequestsModel.RiskControlsChange {
  const diff = newValue.subtract(oldValue);
  const cmp = diff.compare(Nexus.Money.ZERO);
  const direction = toDirection(cmp);
  return {
    type: 'risk_controls',
    name,
    oldValue: oldValue.toString(),
    newValue: newValue.toString(),
    delta: {value: diff.toString(), direction}
  };
}

function toFirstEntitlementChange(current: Beam.Set<Beam.DirectoryEntry>,
    requested: Beam.Set<Beam.DirectoryEntry>,
    entitlementDatabase: Nexus.EntitlementDatabase,
    currencyDatabase: Nexus.CurrencyDatabase):
      RequestsModel.EntitlementsChange {
  for(const entry of requested) {
    if(!current.test(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = entitlementName(entry, info);
      const direction = (() => {
        if(info.price.equals(Nexus.Money.ZERO)) {
          return RequestsModel.Direction.NONE;
        }
        return RequestsModel.Direction.POSITIVE;
      })();
      return {
        type: 'entitlements',
        name,
        action: RequestsModel.EntitlementAction.GRANT,
        fee: info.price,
        currency: currencyDatabase.fromCurrency(info.currency),
        direction
      };
    }
  }
  for(const entry of current) {
    if(!requested.test(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = entitlementName(entry, info);
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
  if(RequestsModel.riskStateToString(current.allowedState) !==
      RequestsModel.riskStateToString(requested.allowedState)) { ++count; }
  return count;
}

function toRiskChanges(current: Nexus.RiskParameters,
    requested: Nexus.RiskParameters,
    currencyDatabase: Nexus.CurrencyDatabase): RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  addRiskChange(changes, 'Currency',
    RequestsModel.currencyName(current.currency, currencyDatabase),
    RequestsModel.currencyName(requested.currency, currencyDatabase));
  addMoneyRiskChange(
    changes, 'Buying Power', current.buyingPower, requested.buyingPower);
  addMoneyRiskChange(
    changes, 'Net Loss', current.netLoss, requested.netLoss);
  addDurationRiskChange(changes, 'Transition Time',
    current.transitionTime, requested.transitionTime);
  addRiskChange(changes, 'Allowed State',
    RequestsModel.riskStateToString(current.allowedState),
    RequestsModel.riskStateToString(requested.allowedState));
  return changes;
}

function addMoneyRiskChange(changes: RequestsModel.DetailChange[],
    name: string, oldValue: Nexus.Money, newValue: Nexus.Money) {
  if(oldValue.equals(newValue)) {
    return;
  }
  const diff = newValue.subtract(oldValue);
  const cmp = diff.compare(Nexus.Money.ZERO);
  const direction = toDirection(cmp);
  changes.push({
    type: 'risk',
    name: name,
    oldValue: oldValue.toString(),
    newValue: newValue.toString(),
    delta: {value: diff.toString(), direction}
  });
}

function addDurationRiskChange(changes: RequestsModel.DetailChange[],
    name: string, oldValue: Beam.Duration, newValue: Beam.Duration) {
  if(oldValue.equals(newValue)) {
    return;
  }
  const diff = newValue.subtract(oldValue);
  const cmp = diff.compare(Beam.Duration.ZERO);
  const direction = toDirection(cmp);
  changes.push({
    type: 'risk',
    name: name,
    oldValue: oldValue.toString(),
    newValue: newValue.toString(),
    delta: {value: diff.toString(), direction}
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

function toEntitlementChanges(current: Beam.Set<Beam.DirectoryEntry>,
    requested: Beam.Set<Beam.DirectoryEntry>,
    entitlementDatabase: Nexus.EntitlementDatabase,
    currencyDatabase: Nexus.CurrencyDatabase):
      RequestsModel.DetailChange[] {
  const changes: RequestsModel.DetailChange[] = [];
  for(const entry of requested) {
    if(!current.test(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = entitlementName(entry, info);
      changes.push({
        type: 'entitlement',
        name,
        oldStatus: RequestsModel.EntitlementStatus.REVOKED,
        newStatus: RequestsModel.EntitlementStatus.GRANTED,
        delta: RequestsModel.entitlementDelta(info, currencyDatabase,
          RequestsModel.EntitlementStatus.GRANTED)
      });
    }
  }
  for(const entry of current) {
    if(!requested.test(entry)) {
      const info = entitlementDatabase.fromGroup(entry);
      const name = entitlementName(entry, info);
      changes.push({
        type: 'entitlement',
        name,
        oldStatus: RequestsModel.EntitlementStatus.GRANTED,
        newStatus: RequestsModel.EntitlementStatus.REVOKED,
        delta: RequestsModel.entitlementDelta(info, currencyDatabase,
          RequestsModel.EntitlementStatus.REVOKED)
      });
    }
  }
  return changes;
}
