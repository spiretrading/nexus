define(function(require) {
  var LoginPage = require('app/pages/LoginPage');
  var React = require('react');
  var ReactDOM = require('react-dom');
  var SpireWebClient = require('app/services/SpireWebClient');
  var client = new SpireWebClient();
  if(client.loadCurrentAccount().type == -1) {
    ReactDOM.render(
      <LoginPage
        client = {client}
        success = {
          function() {
            window.location.href = '/dashboard';
          }
        }
      />, document.getElementById('container'));
  } else {
    window.location.href = '/dashboard';
  }
});
