import serviceLocatorClient from 'utils/spire-clients/service-locator';
import adminClient from 'utils/spire-clients/admin';
import ResultCode from './result-codes';

/** Centralized user related states and service actions */
class UserService {
  constructor() {
    this.roles = {
      isTrader: false,
      isManager: false,
      isService: false,
      isAdmin: false
    };
    this.userName;
    this.directoryEntry;
  }

  initialize(userDirectoryEntry) {
    this.directoryEntry = userDirectoryEntry;
    this.userName = userDirectoryEntry.name;
    return adminClient.loadAccountRoles.apply(adminClient, [this.directoryEntry])
      .then((response) => {
        this.roles = response;
      });
  }

  signIn(userName, password) {
    let resultCode = null;

    return serviceLocatorClient.signIn(userName, password)
      .then(onSignInResponse.bind(this))
      .then(onUserRolesResponse.bind(this))
      .catch(onException);

    function onSignInResponse(response) {
      resultCode = response.resultCode;
      if (resultCode === ResultCode.FAIL) {
        throw resultCode;
      } else {
        this.directoryEntry = response.directoryEntry;
      }

      return adminClient.loadAccountRoles.apply(adminClient, [this.directoryEntry]);
    }

    function onUserRolesResponse(response) {
      this.roles = response;
      this.userName = userName;
      EventBus.publish(Event.Application.SIGNED_IN);
      return resultCode;
    }

    function onException(resultCode) {
      if (resultCode === ResultCode.FAIL) {
        return resultCode;
      } else {
        return ResultCode.ERROR;
      }
    }
  }

  signOut() {
    serviceLocatorClient.signOut();
    this.userName = null;
    this.directoryEntry = null;
  }

  isSignedIn() {
    return this.directoryEntry != null;
  }

  getUserName() {
    return this.userName;
  }

  getDirectoryEntry() {
    return this.directoryEntry;
  }

  isTrader() {
    return this.roles.isTrader;
  }

  isManager() {
    return this.roles.isManager;
  }

  isService() {
    return this.roles.isService;
  }

  isAdmin() {
    return this.roles.isAdmin;
  }

  isAuthorizedPath(path) {
    if (!this.roles.isAdmin && !this.roles.isManager && path === '/searchProfiles'){
      return false;
    } else {
      return true;
    }
  }
}

export default new UserService();
