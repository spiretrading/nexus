import {AdministrationClient, ServiceLocatorClient} from 'spire-client';
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
    this.adminClient = new AdministrationClient();
    this.serviceLocatorClient = new ServiceLocatorClient();
  }

  initialize(userDirectoryEntry) {
    this.directoryEntry = userDirectoryEntry;
    this.userName = userDirectoryEntry.name;
    return this.adminClient.loadAccountRoles.apply(this.adminClient, [this.directoryEntry])
      .then((response) => {
        this.roles = response;
      });
  }

  signIn(userName, password) {
    let resultCode = null;

    return this.serviceLocatorClient.signIn(userName, password)
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

      return this.adminClient.loadAccountRoles.apply(this.adminClient, [this.directoryEntry]);
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
    this.serviceLocatorClient.signOut();
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
