import {
  AdministrationClient,
  AccountModificationRequestType
} from 'spire-client'
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import {browserHistory} from 'react-router';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.requiredDataLoaded = false;

    this.loadModificationRequests = this.loadModificationRequests.bind(this);
    this.onSelect = this.onSelect.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  loadMyAccountRequests() {
    this.getLoadMyAccountRequestsPromise()
      .then(requests => {
        this.componentModel.accountModificationRequests = requests;
        this.view.update(this.componentModel);
      });
  }

  loadManagedAccountModificationRequests() {
    this.getLoadManagedAccountRequestsPromise()
      .then(requests => {
        this.componentModel.accountModificationRequests = requests;
        this.view.update(this.componentModel);
      });
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then(responses => {
      this.componentModel.accountModificationRequests = responses[0];
      this.requiredDataLoaded = true;
      this.componentModel.isAuthority = userService.isManager() || userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isRequiredDataLoaded() {
    return this.requiredDataLoaded;
  }

  onSelect(id, requestType, requesterAccount, changeAccount, requestStatus) {
    if (requestType == AccountModificationRequestType.ENTITLEMENTS) {
      browserHistory.push({
        pathname: '/entitlement-modification-review',
        state: {
          id: id,
          requesterAccount: requesterAccount,
          changeAccount: changeAccount,
          requestStatus: requestStatus
        }
      });
    } else if (requestType == AccountModificationRequestType.RISK_CONTROLS) {
      browserHistory.push({
        pathname: '/risk-controls-modification-review',
        state: {
          id: id
        }
      });
    }
  }

  /** @private */
  getRequiredData() {
    return Promise.all([this.getLoadMyAccountRequestsPromise()]);
  }

  /** @private */
  getLoadMyAccountRequestsPromise() {
    let directoryEntry = userService.getDirectoryEntry();
    return this.adminClient.loadAccountModificationRequestIds(directoryEntry, -1, 10)
      .then(this.loadModificationRequests);
  }

  /** @private */
  getLoadManagedAccountRequestsPromise() {
    let directoryEntry = userService.getDirectoryEntry();
    return this.adminClient.loadManagedAccountModificationRequests(directoryEntry, -1, 10)
      .then(this.loadModificationRequests);
  }

  /** @private */
  loadModificationRequests(ids) {
    let resolve;
    let loadPromise = new Promise((res, rej) => {
      resolve = res;
    });

    let loadRequests = [];
    let loadStatus = [];
    for (let i=0; i<ids.length; i++) {
      loadRequests.push(this.adminClient.loadAccountModificationRequest(ids[i]));
      loadStatus.push(this.adminClient.loadAccountModificationStatus(ids[i]));
    }

    let allRequests = Promise.all(loadRequests);
    let allStatus = Promise.all(loadStatus);

    Promise.all([
      allRequests,
      allStatus
    ]).then(results => {
      let requests = [];
      for (let i=0; i<ids.length; i++) {
        requests.push({
          request: results[0][i],
          update: results[1][i]
        });
      }
      resolve(requests);
    });

    return loadPromise;
  }
}

export default Controller;
