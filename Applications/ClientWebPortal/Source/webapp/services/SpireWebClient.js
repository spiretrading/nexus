define(['jquery'],
  function($) {

    /** Exposes Spire web services */
    class SpireWebClient {

      /** Logs onto the Spire web server. */
      login(username, password) {
        return new Promise(
          function(resolve, reject) {
            var loginRequest = $.ajax(
              {
                url: '/api/service_locator/login',
                dataType: 'json',
                method: 'POST',
                data: JSON.stringify(
                  {
                    username: username,
                    password: password
                  })
              }
            ).done(
              function(data, status, xhr) {
                resolve();
              }
            ).fail(
              function(data, xhr, status, err) {
                reject();
              });
          });
      }
    }
    return SpireWebClient;
  }
);
