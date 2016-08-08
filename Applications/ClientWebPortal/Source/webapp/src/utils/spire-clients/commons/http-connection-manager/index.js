/** Spire HTTP connection manager */
class HTTPConnectionManager {
  send(apiPath, payload, isJsonResponse) {
    return new Promise(function (resolve, reject) {
      let options = {
        url: apiPath,
        method: 'POST'
      };

      if (payload != null) {
        options.contentType = 'application/json; charset=utf-8';
        options.data = JSON.stringify(payload);
      }

      if (isJsonResponse) {
        options.dataType = 'json';
      }

      $.ajax(options)
        .done(onDone)
        .fail(onFail);

      function onDone(data, status, xhr) {
        resolve(data);
      }

      function onFail(xhr) {
        reject(xhr);
      }
    });
  }
}

export default new HTTPConnectionManager();