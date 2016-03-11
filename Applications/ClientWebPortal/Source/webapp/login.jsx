define(
  function(require) {
    var LoginPage = require('app/pages/LoginPage');
    var SpireWebClient = require('app/services/SpireWebClient');
    var client = new SpireWebClient();
    ReactDOM.render(<LoginPage client = {client} />,
      document.getElementById('container'));
  }
);
