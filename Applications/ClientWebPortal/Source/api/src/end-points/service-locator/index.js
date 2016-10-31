import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes';
import accountRoles from '../commons/account-roles';
const ResultCode = ResultCodes;

/** Spire service locator client class */
class ServiceLocator {
  /** @private */
  logErrorAndThrow(xhr) {
    let errorMessage = 'Spire Service Locator Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(xhr);
    throw errorMessage;
  }

  signIn(userName, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userName,
      password: password
    };

    return httpConnectionManager.send(apiPath, payload, true)
      .then(onSuccess, onHttpError.bind(this));

    function onSuccess(directoryEntry) {
      return {
        resultCode: ResultCode.SUCCESS,
        directoryEntry: directoryEntry
      };
    }

    function onHttpError(xhr) {
      if (xhr.status === 401) {
        return {
          resultCode: ResultCode.FAIL
        };
      } else {
        this.logErrorAndThrow(xhr);
      }
    }
  }

  signOut() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/logout';
    return httpConnectionManager.send(apiPath, null, false)
      .catch(this.logErrorAndThrow);
  }

  storePassword(directoryEntry, newPassword) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/store_password';
    let payload = {
      account: directoryEntry,
      password: newPassword
    };
    return httpConnectionManager.send(apiPath, payload, false)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(response) {
      return { resultCode: ResultCode.SUCCESS }
    }
  }

  createAccount(userName, group, accountIdentity, roles) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/create_account';
    let payload = {
      name: userName,
      group: group,
      identity: accountIdentity,
      roles: accountRoles.encode(roles)
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .then(JSON.parse)
      .catch(this.logErrorAndThrow);
  }

  createGroup(groupName) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/create_group';
    let payload = {
      name: groupName
    };

    return httpConnectionManager.send(apiPath, payload, false)
      .catch(this.logErrorAndThrow);
  }

  loadCurrentAccount() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/load_current_account';
    return httpConnectionManager.send(apiPath, null, true)
      .catch(this.logErrorAndThrow);
  }
}

export default ServiceLocator;
