import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
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

const MODEL = new WebPortal.LocalRequestsModel(
  ACCOUNTS[0], makeEntries(500), new Map());

const ROLES = new Nexus.AccountRoles();
ROLES.set(Nexus.AccountRoles.Role.ADMINISTRATOR);

ReactDOM.render(<WebPortal.RequestsController roles={ROLES} model={MODEL}/>,
  document.getElementById('main'));
