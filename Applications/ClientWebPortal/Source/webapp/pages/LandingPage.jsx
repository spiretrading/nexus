define(function(require) {
  var React = require('react');
  var ReactDOM = require('react-dom');
  var $ = require('jquery');
  var Header = require('app/components/Header');

  class LandingPage extends React.Component {
    constructor() {
      super();
      this.state =
        {
          isMenuDisplayed: false
        };
      this.toggleMenu = this.toggleMenu.bind(this);
    }

    toggleMenu() {
      if(this.state.isMenuDisplayed) {
        this.hideMenu();
      } else {
        this.showMenu();
      }
    }

    showMenu() {
      if(this.state.isMenuDisplayed) {
        return;
      }
      this.setState({isMenuDisplayed: true});
    }

    hideMenu() {
      if(!this.state.isMenuDisplayed) {
        return;
      }
      this.setState({isMenuDisplayed: false});
    }

    render() {
      var containerStyle = {
        width: '100%',
        height: '100%',
        margin: 0,
        padding: 0,
        display: 'flex',
        flexDirection: 'row',
        flexWrap: 'nowrap',
      };
      var menuStyle = {
        backgroundColor: 'red',
      };
      var bodyStyle = {
        flex: 1,
        display: 'flex',
        flexDirection: 'column',
        flexWrap: 'nowrap'
      };
      var headerStyle = {
        backgroundColor: 'black',
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content'
      };
      return (
        <div style = {containerStyle}>
          {function() {
            if(this.state.isMenuDisplayed) {
              return (
                <div style = {menuStyle}>
                  <ul>
                    <li><a href="#">Reports</a></li>
                    <li><a href="#">Compliance</a></li>
                    <li><a href="#">Sign Out</a></li>
                  </ul>
                </div>);
            } else {
              return null;
            }
          }.bind(this)()}
          <div style = {bodyStyle}>
            <div style = {headerStyle}>
              <Header onMenuClick = {this.toggleMenu} />
            </div>
          </div>
        </div>);
    }
  }
  return LandingPage;
});
