define(function(require) {
  var React = require('react');
  var ReactDOM = require('react-dom');
  var $ = require('jquery');
  var BurgerButton = require('app/components/BurgerButton');

  class LandingPage extends React.Component {
    constructor() {
      super();
    }

    render() {
      return (
        <div className = "landing-page">
          <BurgerButton
            width = {70}
            height = {64}
            background_color = {'transparent'}
            stroke_color = {'white'}
          />
        </div>);
    }
  }
  return LandingPage;
});
