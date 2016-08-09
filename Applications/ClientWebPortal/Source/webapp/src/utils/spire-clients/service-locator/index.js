import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
const ResultCode = ResultCodes;

/** Spire service locator client class */
class ServiceLocatorClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Spire Service Locator Client: Unexpected error happened.');
    throw ResultCode.ERROR;
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
      .catch(this.logErrorAndThrow);
  }
}

export default new ServiceLocatorClient();