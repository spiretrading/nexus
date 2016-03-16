define(['jquery'],
  function($) {

    /** Exposes Spire web services */
    class SpireWebClient {

      /** Loads the DirectoryEntry of the account currently logged in. */
      loadCurrentAccount() {
        var result = null;
        $.ajax(
          {
            async: false,
            url: '/api/service_locator/load_current_account',
            dataType: 'json',
            method: 'POST',
            success:
              function(data, status, xhr) {
                result = data;
              },
            error:
              function(data, xhr, status, err) {
                result =
                  {
                    id: -1,
                    type: -1,
                    name: ''
                  };
              }
          });
        return result;
      }

      /** Logs onto the Spire web server. */
      login(username, password) {
        return new Promise(
          function(resolve, reject) {
            var request = $.ajax(
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
                resolve(data);
              }.bind(this)
            ).fail(
              function(data, xhr, status, err) {
                reject('Invalid username or password.');
              });
          }.bind(this));
      }

      /** Logs out of the Spire web server. */
      logout() {
        return new Promise(
          function(resolve, reject) {
            var request = $.ajax(
              {
                url: '/api/service_locator/logout',
                method: 'POST'
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
