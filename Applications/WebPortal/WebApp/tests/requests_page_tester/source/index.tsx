import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const USD_CURRENCY = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', '$');

const SAMPLE_CHANGES: WebPortal.RequestItem.Change[] = [
  {type: 'entitlements', name: 'NYSE Arca Equities',
    action: WebPortal.EntitlementsChangeItem.Action.GRANT,
    fee: Nexus.Money.parse('14.50'), currency: USD_CURRENCY},
  {type: 'entitlements', name: 'OPRA',
    action: WebPortal.EntitlementsChangeItem.Action.REVOKE,
    fee: Nexus.Money.parse('7.00'), currency: USD_CURRENCY},
  {type: 'risk_controls', name: 'Buying Power',
    oldValue: '$100,000', newValue: '$150,000',
    delta: {value: '$50,000',
      direction: WebPortal.DiffBadge.Direction.POSITIVE}},
  {type: 'risk_controls', name: 'Net Loss',
    oldValue: '$5,000', newValue: '$3,000',
    delta: {value: '$2,000',
      direction: WebPortal.DiffBadge.Direction.NEGATIVE}}
];

const SAMPLE_REQUEST_LIST:
    WebPortal.RequestDirectoryPage.RequestEntry[] =
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

const ROLES = new Nexus.AccountRoles();
ROLES.set(Nexus.AccountRoles.Role.MANAGER);

ReactDOM.render(
  <WebPortal.RequestsPage
    roles={ROLES}
    displayStatus={WebPortal.RequestDirectoryPage.DisplayStatus.READY}
    requestState={WebPortal.RequestDirectoryPage.RequestState.PENDING}
    filters={{
      query: '',
      categories: new Set<Nexus.AccountModificationRequest.Type>(),
      sortKey: WebPortal.RequestSortSelect.Field.LAST_UPDATED
    }}
    filterCount={0}
    pageIndex={0}
    response={{
      status: WebPortal.RequestDirectoryPage.ResponseStatus.READY,
      facetCounts: {pending: 5, approved: 122, rejected: 3},
      requestList: SAMPLE_REQUEST_LIST
    }}/>,
  document.getElementById('main'));
