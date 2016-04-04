define(function(require) {
  var React = require('react');
  var ReactDOM = require('react-dom');
  var $ = require('jquery');
  var Header = require('app/components/Header');

  class LandingPage extends React.Component {
    constructor() {
      super();
    }

    render() {
      var style = {
        color: 'white',
        background: 'white',
        opacity: 1
      };
      return (
        <div style = {style}>
          <Header />
        </div>);
    }
  }
  return LandingPage;
});
