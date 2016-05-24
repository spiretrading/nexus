define(['jquery'],
  function($) {

    /** Exposes Spire's administration services. */
    class AdministrationClient {

      /** Returns the list of trading groups managed by an account. */
      loadManagedTradingGroups(account) {
        return new Promise(
          function(resolve, reject) {
            var request = $.ajax(
              {
                url: '/api/administration_service/load_managed_trading_groups',
                dataType: 'json',
                method: 'POST',
                data: JSON.stringify(
                  {
                    account: account,
                  })
              }
            ).done(
              function(data, status, xhr) {
                resolve(data);
              }
            ).fail(
              function(data, xhr, status, err) {
                reject();
              });
          });
      }
    }
    return AdministrationClient;
  }
);
