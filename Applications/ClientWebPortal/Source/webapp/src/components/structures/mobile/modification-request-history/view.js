import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import AccModRequest from 'components/reusables/mobile/account-modification-request';
import { AccountModificationRequestStatus } from 'spire-client';
import modal from 'utils/modal';

const FILTER_PENDING = 'pending';
const FILTER_APPROVED = 'approved';
const FILTER_REJECTED = 'rejected';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onStatusFilterChange = this.onStatusFilterChange.bind(this);
    this.onMyAccountsClick = this.onMyAccountsClick.bind(this);
    this.onManagedAccountsClick = this.onManagedAccountsClick.bind(this);
    this.onFilterClick = this.onFilterClick.bind(this);

    this.isMyAccounts = true;
    this.filterStatus = {};
    this.filterStatus[FILTER_PENDING] = true;
    this.filterStatus[FILTER_APPROVED] = true;
    this.filterStatus[FILTER_REJECTED] = true;
  }

  componentDidUpdate() {
    $('#modification-request-history-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  /** @private */
  onStatusFilterChange(e) {
    let $wrapper = $(e.currentTarget).parent();
    if ($wrapper.hasClass('selected')) {
      $wrapper.removeClass('selected');
      this.filterStatus[$wrapper.attr('data-filter')] = false;
    } else {
      $wrapper.removeClass('selected').addClass('selected');
      this.filterStatus[$wrapper.attr('data-filter')] = true;
    }
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
    if (this.filterStatus[FILTER_PENDING]) {
      if (requestStatus == AccountModificationRequestStatus.PENDING ||
          requestStatus == AccountModificationRequestStatus.REVIEWED) {
        return true;
      }
    }
    if (this.filterStatus[FILTER_APPROVED]) {
      if (requestStatus == AccountModificationRequestStatus.SCHEDULED ||
          requestStatus == AccountModificationRequestStatus.GRANTED) {
        return true;
      }
    }
    if (this.filterStatus[FILTER_REJECTED]) {
      if (requestStatus == AccountModificationRequestStatus.REJECTED) {
        return true;
      }
    }

    return false;
  }

  /** @private */
  onFilterClick() {
    modal.show($('#filters-modal'));
  }

  /** @private */
  onModalCloseClick() {
    modal.hide($('#filters-modal'));
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

      let filtersWrapperClass = "filters-wrapper";
      if (requestPanels.length === 0) {
        filtersWrapperClass += ' no-items';
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
          <div className={filtersWrapperClass}>
            <div className="account-filter-wrapper">
              <div className={myAccountsClass} onClick={this.onMyAccountsClick}>
                My Account
              </div>
              <div className={managedAccountsClass} onClick={this.onManagedAccountsClick}>
                Managed Accounts
              </div>
              <span className="icon-filter" onClick={this.onFilterClick}></span>
            </div>
          </div>
          <div className="requests-wrapper">
            {requestPanels}
          </div>

          <div id="filters-modal" className="modal fade" tabIndex="-1" role="dialog">
            <div className="modal-dialog" role="document">
              <div className="modal-content">
                <div className="modal-header">
                  Filters
                  <span className="icon-close" onClick={this.onModalCloseClick}></span>
                </div>
                <div className="modal-body">
                  <div className="selected menu-item" data-filter={FILTER_PENDING}>
                    <span className="icon-check" onClick={this.onStatusFilterChange} />
                    <span className="text-label">Pending</span>
                  </div>
                  <div className="selected menu-item" data-filter={FILTER_APPROVED}>
                    <span className="icon-check" onClick={this.onStatusFilterChange} />
                    <span className="text-label">Approved</span>
                  </div>
                  <div className="selected menu-item" data-filter={FILTER_REJECTED}>
                    <span className="icon-check" onClick={this.onStatusFilterChange} />
                    <span className="text-label">Rejected</span>
                  </div>
                </div>
              </div>
            </div>
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
