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
      var burgerStyle = {
        float: 'left',
      };
      return (
        <div style = {burgerStyle}>
          <BurgerButton
            width = {30}
            height = {30}
            background_color = {'transparent'}
            stroke_color = {'white'}
            onClick = {this.props.onMenuClick}
          />
        </div>);
    }
  }
  return Header;
});
