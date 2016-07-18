/** Spire HTTP connection manager */
class HTTPConnectionManager {
  send(apiPath, payload) {
    return new Promise(function (resolve, reject) {
      let options = {
        url: apiPath,
        method: 'POST',
        contentType: 'application/json; charset=utf-8'
      };

      if (payload != null) {
        options.dataType = 'json';
        options.data = JSON.stringify(payload);
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