define(function(require) {
  var LoginPage = require('app/pages/LoginPage');
  var LandingPage = require('app/pages/LandingPage');
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
            ReactDOM.render(
              <LandingPage
                client = {client}
              />, document.getElementById('container'));
          }
        }
      />, document.getElementById('container'));
  } else {
    ReactDOM.render(
      <LandingPage
        client = {client}
      />, document.getElementById('container'));
  }
});
