import ResultCode from './result-codes.js';

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

      console.log(jsonPayload);

      /*
      var request = $.ajax({
        url: apiPath,
        dataType: 'json',
        method: 'POST',
        data: jsonPayload
      }).done(function (data, status, xhr) {
        console.log('succeded');
        resolve(data);
      }.bind(this)).fail(function (xhr, status, error) {
        console.log('failed');
        reject(xhr, status, error);
      });
      */

      return new Promise(
          function(resolve, reject) {
            var request = $.ajax(
                {
                  url: apiPath,
                  dataType: 'json',
                  method: 'POST',
                  data: jsonPayload
                }
            ).done(
                function(data, status, xhr) {
                  console.log('succeded');
                  resolve(data);
                }.bind(this)
            ).fail(
                function(data, xhr, status, err) {
                  console.log('failed');
                  reject('Invalid username or password.');
                });
          }.bind(this));

    }.bind(this));
  }

  login(userId, password) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'service_locator/login';
    let payload = {
      username: userId,
      password: password
    };

    return this.send(apiPath, payload)
      .then(onSuccess, onHttpError);

    function onSuccess(){
      return ResultCode.Success;
    }

    function onHttpError(xhr, status, error){
      if (true){
        return ResultCode.Fail;
      }

      // server error
      else{
        console.log('Unexpected error happened.');
        throw new Error();
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