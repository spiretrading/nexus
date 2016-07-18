import httpConnectionManager from './http-connection-manager';
import ResultCodes from './result-codes.js';
const ResultCode = ResultCodes;

/** Spire client class */
class SpireClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.log('Unexpected error happened.');
    throw ResultCode.ERROR;
  }

  signIn(userId, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userId,
      password: password
    };

    return httpConnectionManager.send(apiPath, payload)
      .then(onSuccess, onHttpError.bind(this));

    function onSuccess(param) {
      return ResultCode.SUCCESS;
    }

    function onHttpError(xhr) {
      if (xhr.status === 401) {
        return ResultCode.FAIL;
      } else {
        this.logErrorAndThrow(xhr);
      }
    }
  }

  signOut() {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/logout';
    return httpConnectionManager.send(apiPath, null)
      .catch(this.logErrorAndThrow);
  }

  getUserRole(userId) {
    return new Promise((resolve, reject) => {
      resolve(0);
    });
  }
}

export default new SpireClient();