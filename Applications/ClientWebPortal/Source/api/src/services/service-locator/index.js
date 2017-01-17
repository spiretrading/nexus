import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes';
import accountRoles from '../commons/account-roles';
import DirectoryEntry from './directory-entry';
const ResultCode = ResultCodes;

/** Spire service locator client class */
class ServiceLocator {
  /** @private */
  logErrorAndThrow(error) {
    let errorMessage = 'Spire Service Locator Client: Unexpected error happened.';
    console.error(errorMessage);
    console.error(error);
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

    function onSuccess(directoryEntryData) {
      return {
        resultCode: ResultCode.SUCCESS,
        directoryEntry: DirectoryEntry.fromData(directoryEntryData)
      };
    }

    function onHttpError(errorCode) {
      if (errorCode.message == 401) {
        return {
          resultCode: ResultCode.FAIL
        };
      } else {
        this.logErrorAndThrow('HTTP Error: ' + errorCode.message);
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
      account: directoryEntry.toData(),
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
      identity: accountIdentity.toData(),
      roles: accountRoles.encode(roles)
    };

    return httpConnectionManager.send(apiPath, payload, true)
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

  searchDirectoryEntry(prefix) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/search_directory_entry';
    let payload = {
      name: prefix
    };
    return httpConnectionManager.send(apiPath, payload, true)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(results) {
      for (let i=0; i<results.length; i++) {
        let roles = results[i].roles;
        results[i].roles = accountRoles.parse(roles);
      }

      return results;
    }
  }
}

export default ServiceLocator;
