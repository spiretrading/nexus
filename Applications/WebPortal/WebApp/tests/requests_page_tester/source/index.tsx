import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { HashRouter, Redirect, Route, Switch } from 'react-router-dom';
import * as WebPortal from 'web_portal';

type Type = Nexus.AccountModificationRequest.Type;
type Status = Nexus.AccountModificationRequest.Status;
const Type = Nexus.AccountModificationRequest.Type;
const Status = Nexus.AccountModificationRequest.Status;
const Direction = WebPortal.RequestsModel.Direction;
const EntitlementAction = WebPortal.RequestsModel.EntitlementAction;

const USD = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', '$');
const CAD = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(124), 'CAD', '$');

const ACCOUNT_NAMES = [
  'achen01', 'bsmith02', 'cgreen01', 'djones04', 'ewilson05',
  'fgarcia06', 'hlee07', 'jmartin08', 'kpatel09', 'lnguyen10',
  'mrobinson11', 'nthompson12', 'oanderson13', 'ptaylor14', 'qjackson15'
];
const ACCOUNTS = ACCOUNT_NAMES.map(
  (name, i) => Beam.DirectoryEntry.makeAccount(100 + i, name));

const ENTITLEMENT_NAMES = [
  'NYSE Arca Equities', 'OPRA', 'NYSE American Equities',
  'NASDAQ TotalView', 'CME Group', 'CBOE Equities',
  'TSX Venture', 'ICE Futures'
];

const ENTITLEMENT_CHANGES: WebPortal.RequestsModel.ListChange[] =
  ENTITLEMENT_NAMES.map((name, i) => ({
    type: 'entitlements' as const,
    name,
    action: i % 3 === 0 ? EntitlementAction.REVOKE : EntitlementAction.GRANT,
    fee: Nexus.Money.parse(String((i + 1) * 7)),
    currency: i % 4 === 0 ? CAD : USD
  }));

function makeMoneyRiskChange(name: string):
    WebPortal.RequestsModel.ListChange {
  const oldVal = 50000 + Math.floor(Math.random() * 100000);
  const newVal = oldVal + 10000 + Math.floor(Math.random() * 40000);
  return {
    type: 'risk_controls' as const,
    name,
    oldValue: `$${oldVal.toLocaleString()}`,
    newValue: `$${newVal.toLocaleString()}`,
    delta: {
      value: `$${(newVal - oldVal).toLocaleString()}`,
      direction: Direction.POSITIVE
    }
  };
}

const RISK_CHANGES: WebPortal.RequestsModel.ListChange[] = [
  makeMoneyRiskChange('Buying Power'),
  makeMoneyRiskChange('Net Loss'),
  {
    type: 'risk_controls' as const,
    name: 'Transition Time',
    oldValue: '30 minutes',
    newValue: '45 minutes',
    delta: {
      value: '15 minutes',
      direction: Direction.POSITIVE
    }
  }
];

const ALL_CHANGES = [...ENTITLEMENT_CHANGES, ...RISK_CHANGES];

const STATUSES: Status[] = [
  Status.PENDING, Status.PENDING, Status.PENDING,
  Status.REVIEWED, Status.SCHEDULED,
  Status.GRANTED, Status.GRANTED,
  Status.REJECTED
];

function makeEntries(count: number):
    WebPortal.RequestsModel.RequestEntry[] {
  const entries: WebPortal.RequestsModel.RequestEntry[] = [];
  const baseDate = new Date(2025, 0, 15);
  for(let i = 0; i < count; ++i) {
    const change = ALL_CHANGES[i % ALL_CHANGES.length];
    const account = ACCOUNTS[i % ACCOUNTS.length];
    const status = STATUSES[i % STATUSES.length];
    const requestDate = new Date(baseDate);
    requestDate.setDate(baseDate.getDate() + i);
    const updateDate = new Date(requestDate);
    updateDate.setDate(requestDate.getDate() + 1 + (i % 5));
    const effectiveDate = new Date(updateDate);
    effectiveDate.setDate(updateDate.getDate() + 7 + (i % 14));
    const additionalChanges = i % 5 === 0 ? 3 : i % 7 === 0 ? 1 : 0;
    const comments = i % 4 === 0 ? 2 : i % 6 === 0 ? 1 : 0;
    const approval = status === Status.REVIEWED ?
      {approver: ACCOUNTS[(i + 3) % ACCOUNTS.length].name, self: false} :
      undefined;
    entries.push({
      id: 1000 + i,
      category: change.type === 'entitlements' ?
        Type.ENTITLEMENTS : Type.RISK,
      state: status,
      updateTime: updateDate,
      account: account,
      effectiveDate: effectiveDate,
      firstChange: change,
      additionalChangesCount: additionalChanges,
      commentCount: comments,
      managerApproval: approval
    });
  }
  return entries;
}

const AVATAR_TINTS = [
  '#F44336', '#9C27B0', '#3F51B5', '#009688', '#FF9800',
  '#795548', '#607D8B', '#E91E63', '#673AB7', '#2196F3'
];

function makeProfile(
    account: Beam.DirectoryEntry): WebPortal.RequestsModel.AccountProfile {
  const name = account.name;
  const initials = name.substring(0, 2).toUpperCase();
  const tint = AVATAR_TINTS[account.id % AVATAR_TINTS.length];
  return {account, initials, tint};
}

const EntitlementStatus = WebPortal.RequestsModel.EntitlementStatus;

function toDetailChanges(
    entry: WebPortal.RequestsModel.RequestEntry):
    WebPortal.RequestsModel.DetailChange[] {
  const changes: WebPortal.RequestsModel.DetailChange[] = [];
  const firstChange = entry.firstChange;
  if(firstChange.type === 'entitlements') {
    changes.push({
      type: 'entitlement' as const,
      name: firstChange.name,
      oldStatus: firstChange.action === EntitlementAction.GRANT ?
        EntitlementStatus.REVOKED : EntitlementStatus.GRANTED,
      newStatus: firstChange.action === EntitlementAction.GRANT ?
        EntitlementStatus.GRANTED : EntitlementStatus.REVOKED,
      delta: {
        value: `${firstChange.currency.sign}${firstChange.fee}`,
        direction: firstChange.action === EntitlementAction.GRANT ?
          Direction.POSITIVE : Direction.NEGATIVE
      }
    });
  } else {
    changes.push({
      type: 'risk' as const,
      name: firstChange.name,
      oldValue: firstChange.oldValue,
      newValue: firstChange.newValue,
      delta: firstChange.delta
    });
  }
  const pool = firstChange.type === 'entitlements' ?
    ENTITLEMENT_CHANGES : RISK_CHANGES;
  for(let j = 0; j < entry.additionalChangesCount; ++j) {
    const extra = pool[(entry.id + j + 1) % pool.length];
    if(extra.type === 'entitlements') {
      changes.push({
        type: 'entitlement' as const,
        name: extra.name,
        oldStatus: extra.action === EntitlementAction.GRANT ?
          EntitlementStatus.REVOKED : EntitlementStatus.GRANTED,
        newStatus: extra.action === EntitlementAction.GRANT ?
          EntitlementStatus.GRANTED : EntitlementStatus.REVOKED
      });
    } else {
      changes.push({
        type: 'risk' as const,
        name: extra.name,
        oldValue: extra.oldValue,
        newValue: extra.newValue,
        delta: extra.delta
      });
    }
  }
  return changes;
}

function makeDetails(entries: WebPortal.RequestsModel.RequestEntry[]):
    Map<number, WebPortal.RequestsModel.RequestDetail> {
  const details = new Map<number, WebPortal.RequestsModel.RequestDetail>();
  for(const entry of entries) {
    const requester = ACCOUNTS[(entry.id + 5) % ACCOUNTS.length];
    const activityList: WebPortal.RequestsModel.ActivityEntry[] = [
      {
        account: makeProfile(requester),
        activity: Status.PENDING,
        timestamp: entry.updateTime
      }
    ];
    if(entry.commentCount > 0) {
      for(let c = 0; c < entry.commentCount; ++c) {
        const commenter = ACCOUNTS[(entry.id + c + 2) % ACCOUNTS.length];
        activityList.push({
          account: makeProfile(commenter),
          activity: 'Reviewed the changes and left feedback.',
          timestamp: new Date(
            entry.updateTime.getTime() + (c + 1) * 3600000)
        });
      }
    }
    if(entry.state === Status.GRANTED) {
      activityList.push({
        account: makeProfile(ACCOUNTS[0]),
        activity: Status.GRANTED,
        timestamp: new Date(entry.updateTime.getTime() + 86400000)
      });
    } else if(entry.state === Status.REJECTED) {
      activityList.push({
        account: makeProfile(ACCOUNTS[0]),
        activity: Status.REJECTED,
        timestamp: new Date(entry.updateTime.getTime() + 86400000)
      });
    }
    details.set(entry.id, {
      id: entry.id,
      category: entry.category,
      state: entry.state,
      createdTime: new Date(
        entry.updateTime.getTime() - 86400000),
      updateTime: entry.updateTime,
      account: makeProfile(entry.account),
      requester: makeProfile(requester),
      effectiveDate: Beam.Date.fromDate(entry.effectiveDate),
      changes: toDetailChanges(entry),
      activityList,
      accessRole: Nexus.AccountRoles.Role.ADMINISTRATOR
    });
  }
  return details;
}

const ENTRIES = makeEntries(500);
const MODEL = new WebPortal.LocalRequestsModel(
  ACCOUNTS[0], ENTRIES, makeDetails(ENTRIES));

const ROLES = new Nexus.AccountRoles();
ROLES.set(Nexus.AccountRoles.Role.ADMINISTRATOR);

ReactDOM.render(
  <HashRouter>
    <Switch>
      <Route path='/requests/:tab'
        render={(props) =>
          <WebPortal.RequestsController {...props}
            roles={ROLES} model={MODEL}/>}/>
      <Redirect to='/requests/you'/>
    </Switch>
  </HashRouter>,
  document.getElementById('main'));
