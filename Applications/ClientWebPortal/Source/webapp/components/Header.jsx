define(function(require) {
  var React = require('react');
  var ReactDOM = require('react-dom');
  var $ = require('jquery');
  var BurgerButton = require('app/components/BurgerButton');

  class Header extends React.Component {
    constructor() {
      super();
    }

    render() {
      var leftStyle = {
        position: 'absolute',
        left: 0,
        top: 0
      };
      return (
        <div>
          <div style = {leftStyle}>
            <BurgerButton
              width = {70}
              height = {64}
              background_color = {'transparent'}
              stroke_color = {'white'}
            />
          </div>
        </div>);
    }
  }
  return Header;
});
