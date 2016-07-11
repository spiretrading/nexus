import ResultCode from './result-codes.js';

class SpireClient {
  /** @private */
  send(apiPath, payload) {
    return new Promise((resolve, reject) => {
      let jsonPayload;
      if (payload != null) {
        jsonPayload = JSON.stringify(payload);
      }
      else {
        jsonPayload = null;
      }

      console.log(jsonPayload);

      $.ajax({
        url: apiPath,
        contentType: "application/json; charset=utf-8",
        dataType: 'json',
        method: 'POST',
        data: jsonPayload,
        success: (data, status, xhr) => {
          console.log('succeded');
          resolve(data);
        },
        error: (xhr, status, error) => {
          console.log('failed');
          console.log(xhr.status);
          console.log(status);
          console.log(error);
          reject(xhr, status, error);
        }
      });
    });
  }

  login(userId, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userId,
      password: password
    };

    /*
     return send(apiPath, payload)
     .then(onSuccess, onHttpError);

     function onSuccess(){
     return ResultCode.Success;
     }

     function onHttpError(xhr, status, error){
     if (isLegitimateLoginFail){
     return ResultCode.Fail;
     }
     // server error
     else{
     console.log('Unexpected error happened.');
     throw new Error();
     }
     }
     */

    return new Promise((resolve, reject) => {
      resolve(ResultCode.SUCCESS);
    });
  }

  getUserRole(userId) {
    return new Promise((resolve, reject) => {
      resolve(0);
    });
  }
}

export default new SpireClient();