import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const USD_CURRENCY = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', '$');

const SAMPLE_CHANGES: WebPortal.RequestsModel.ListChange[] = [
  {type: 'entitlements', name: 'NYSE Arca Equities',
    action: WebPortal.RequestsModel.EntitlementAction.GRANT,
    fee: Nexus.Money.parse('14.50'), currency: USD_CURRENCY},
  {type: 'entitlements', name: 'OPRA',
    action: WebPortal.RequestsModel.EntitlementAction.REVOKE,
    fee: Nexus.Money.parse('7.00'), currency: USD_CURRENCY},
  {type: 'risk_controls', name: 'Buying Power',
    oldValue: '$100,000', newValue: '$150,000',
    delta: {value: '$50,000',
      direction: WebPortal.RequestsModel.Direction.POSITIVE}},
  {type: 'risk_controls', name: 'Net Loss',
    oldValue: '$5,000', newValue: '$3,000',
    delta: {value: '$2,000',
      direction: WebPortal.RequestsModel.Direction.NEGATIVE}}
];

const SAMPLE_ENTRIES: WebPortal.RequestsModel.RequestEntry[] =
  SAMPLE_CHANGES.map((change, i) => ({
    id: 1024 + i,
    category: change.type === 'entitlements' ?
      Nexus.AccountModificationRequest.Type.ENTITLEMENTS :
      Nexus.AccountModificationRequest.Type.RISK,
    state: i === 1 ?
      Nexus.AccountModificationRequest.Status.REVIEWED :
      Nexus.AccountModificationRequest.Status.PENDING,
    updateTime: new Date(2025, 8, 23),
    accountName: 'achen01',
    effectiveDate: new Date(2025, 9, 30),
    firstChange: change,
    additionalChangesCount: i === 0 ? 3 : 0,
    commentCount: i === 0 ? 2 : 0,
    managerApproval: i === 1 ?
      {approver: 'cgreen01', self: false} : undefined
  }));

const MODEL = new WebPortal.LocalRequestsModel(SAMPLE_ENTRIES, new Map());

const ROLES = new Nexus.AccountRoles();
ROLES.set(Nexus.AccountRoles.Role.ADMINISTRATOR);

ReactDOM.render(
  <WebPortal.RequestsController roles={ROLES} model={MODEL}/>,
  document.getElementById('main'));
