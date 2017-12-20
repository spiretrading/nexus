import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import AccModRequest from 'components/reusables/common/account-modification-request';
import { AccountModificationRequestStatus } from 'spire-client';

const FILTER_ALL = 'all';
const FILTER_PENDING = 'pending';
const FILTER_APPROVED = 'approved';
const FILTER_REJECTED = 'rejected';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onStatusFilterChange = this.onStatusFilterChange.bind(this);
    this.onMyAccountsClick = this.onMyAccountsClick.bind(this);
    this.onManagedAccountsClick = this.onManagedAccountsClick.bind(this);

    this.isMyAccounts = true;
    this.filterStatus = FILTER_ALL;
  }

  componentDidUpdate() {
    $('#modification-request-history-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  /** @private */
  onStatusFilterChange(e) {
    this.filterStatus = $(e.currentTarget).val();
    this.update();
  }

  /** @private */
  onMyAccountsClick() {
    $('#modification-request-history-container .menu-item.managed-accounts').removeClass('selected');
    $('#modification-request-history-container .menu-item.my-account').removeClass('selected').addClass('selected');
    this.controller.loadMyAccountRequests();
  }

  /** @private */
  onManagedAccountsClick() {
    $('#modification-request-history-container .menu-item.my-account').removeClass('selected');
    $('#modification-request-history-container .menu-item.managed-accounts').removeClass('selected').addClass('selected');
    this.controller.loadManagedAccountModificationRequests();
  }

  /** @private */
  shouldBeIncluded(requestStatus) {
    if (this.filterStatus == FILTER_ALL) {
      return true;
    } else if (this.filterStatus == FILTER_PENDING) {
      if (requestStatus == AccountModificationRequestStatus.PENDING ||
          requestStatus == AccountModificationRequestStatus.REVIEWED) {
        return true;
      }
    } else if (this.filterStatus == FILTER_APPROVED) {
      if (requestStatus == AccountModificationRequestStatus.SCHEDULED ||
          requestStatus == AccountModificationRequestStatus.GRANTED) {
        return true;
      }
    } else if (this.filterStatus == FILTER_REJECTED) {
      if (requestStatus == AccountModificationRequestStatus.REJECTED) {
        return true;
      }
    }

    return false;
  }

  render() {
    let content;

    if (this.controller.isRequiredDataLoaded()) {
      let requestPanels = [];
      for (let i=0; i<this.componentModel.accountModificationRequests.length; i++) {
        let requestStatus = this.componentModel.accountModificationRequests[i].update.status;
        if (this.shouldBeIncluded(requestStatus)) {
          requestPanels.push(
            <div key={i} className="request-wrapper">
              <AccModRequest
                request = {this.componentModel.accountModificationRequests[i].request}
                update = {this.componentModel.accountModificationRequests[i].update}
                onClick = {this.controller.onSelect}
              />
            </div>
          );
        }
      }

      let myAccountsClass = 'menu-item no-select my-account';
      let managedAccountsClass = 'menu-item no-select managed-accounts';
      if (this.isMyAccounts) {
        myAccountsClass += ' selected';
      } else {
        managedAccountsClass += ' selected';
      }

      content =
        <div>
          <div className="page-top-header row">
            Request History
          </div>
          <div className="filters-wrapper">
            <div className="status-filter-wrapper">
              <div>
                Filter Status by
              </div>
              <div className="select-wrapper">
                <select className="status-filter-input" defaultValue={'all'} onChange={this.onStatusFilterChange}>
                  <option value={FILTER_ALL}>All</option>
                  <option value={FILTER_PENDING}>Pending</option>
                  <option value={FILTER_APPROVED}>Approved</option>
                  <option value={FILTER_REJECTED}>Rejected</option>
                </select>
                <span className="icon-arrow-down"/>
              </div>
            </div>
            <div className="account-filter-wrapper">
              <div className={myAccountsClass} onClick={this.onMyAccountsClick}>
                My Accounts
              </div>
              <div className={managedAccountsClass} onClick={this.onManagedAccountsClick}>
                Managed Accounts
              </div>
            </div>
          </div>
          <div className="requests-wrapper">
            {requestPanels}
          </div>
        </div>;
    }

    return (
      <div id="modification-request-history-container">
        {content}
      </div>
    );
  }
}

export default View;
