import httpConnectionManager from './http-connection-manager';
import ResultCodes from './result-codes.js';
const ResultCode = ResultCodes;

/** Spire client class */
class SpireClient {
  login(userId, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userId,
      password: password
    };

    return httpConnectionManager.send(apiPath, payload)
      .then(onSuccess, onHttpError);

    function onSuccess(param) {
      return ResultCode.SUCCESS;
    }

    function onHttpError(xhr) {
      if (xhr.status === 401) {
        return ResultCode.FAIL;
      }
      // TODO: temporary while back-end message is recognized as 200 for some reason
      if (xhr.status === 200) {
        return ResultCode.FAIL;
      }

      // server error
      else {
        console.log('Unexpected error happened.');
        throw ResultCode.ERROR;
      }
    }
  }

  getUserRole(userId) {
    return new Promise((resolve, reject) => {
      resolve(0);
    });
  }
}

export default new SpireClient();