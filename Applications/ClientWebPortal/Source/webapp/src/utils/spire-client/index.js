import ResultCodes from './result-codes.js';
const ResultCode = ResultCodes;

/** Spire client class */
class SpireClient {
  /** @private */
  send(apiPath, payload) {
    return new Promise(function(resolve, reject) {
      let jsonPayload;
      if (payload != null) {
        jsonPayload = JSON.stringify(payload);
      }
      else {
        jsonPayload = null;
      }

      $.ajax({
          url: apiPath,
          dataType: 'json',
          method: 'POST',
          contentType: "application/json; charset=utf-8",
          data: jsonPayload
      })
      .done(onDone)
      .fail(onFail);

      function onDone(data, status, xhr){
        resolve(data);
      }

      function onFail(xhr){
        reject(xhr);
      }
    });
  }

  login(userId, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userId,
      password: password
    };

    return this.send(apiPath, payload)
      .then(onSuccess, onHttpError);

    function onSuccess(param){
      return ResultCode.SUCCESS;
    }

    function onHttpError(xhr){
      if (xhr.status === 401){
        return ResultCode.FAIL;
      }
      // TODO: temporary while back-end message is recognized as 200 for some reason
      if (xhr.status === 200){
        return ResultCode.FAIL;
      }

      // server error
      else{
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