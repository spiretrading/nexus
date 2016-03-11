define(['jquery'],
  function($) {

    /** Exposes Spire web services */
    class SpireWebClient {

      /** Constructs a SpireWebClient. */
      constructor() {
        this.account_ = null;
      }

      /** Returns the account. */
      get account() {
        return this.account_;
      }

      /** Checks if the user is logged in. */
      checkLoggedIn() {
        return false;
      }

      /** Logs onto the Spire web server. */
      login(username, password) {
        if(this.account != null) {
          return new Promise(
            function(resolve, reject) {
              resolve(this.account);
            }.bind(this));
        }
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
                this.account_ = data;
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
        if(this.account == null) {
          return new Promise(
            function(resolve, reject) {
              resolve();
            });
        }
        return new Promise(
          function(resolve, reject) {
            var request = $.ajax(
              {
                url: '/api/service_locator/logout',
                method: 'POST'
              }
            ).done(
              function(data, status, xhr) {
                this.account_ = null;
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
