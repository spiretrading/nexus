/** Spire HTTP connection manager */
class HTTPConnectionManager {
  send(apiPath, payload) {
    return new Promise(function (resolve, reject) {
      let jsonPayload;
      if (payload != null) {
        jsonPayload = JSON.stringify(payload);
      } else {
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