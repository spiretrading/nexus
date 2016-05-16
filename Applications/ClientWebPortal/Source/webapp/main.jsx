define(function(require) {
  var ApplicationPage = require('app/pages/ApplicationPage');
  var React = require('react');
  var ReactDOM = require('react-dom');
  ReactDOM.render(<ApplicationPage />, document.getElementById('container'));
});
