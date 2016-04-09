define(function(require) {
  var LoginPage = require('app/pages/LoginPage');
  var LandingPage = require('app/pages/LandingPage');
  var React = require('react');
  var ReactDOM = require('react-dom');
  var SpireWebClient = require('app/services/SpireWebClient');

  class ApplicationPage extends React.Component {
    constructor() {
      super();
      this.handleLoggedIn = this.handleLoggedIn.bind(this);
      this.client_ = new SpireWebClient();
      this.states_ = new Map();
      this.state_id_ = 0;
      var isLoggedIn = this.client_.loadCurrentAccount().type != -1
      var subComponent;
      if(isLoggedIn) {
        subComponent = <LandingPage application = {this} />;
      } else {
        subComponent = (
          <LoginPage
            application = {this}
            success = {this.handleLoggedIn}
          />);
      }
      this.state = {
        isLoggedIn : isLoggedIn,
        subComponent : subComponent
      };
      window.addEventListener('popstate', function(event) {
        return this.restoreHistory(event);
      }.bind(this));
    }

    get client() {
      return this.client_;
    }

    advanceHistory(component, url) {
      var state_id = this.state_id_;
      ++this.state_id_;
      this.states_.set(state_id, [component, component.state]);
      history.pushState(state_id, '', url);
    }

    restoreHistory(event) {
      var state = this.states_.get(event.state);
      if(state == undefined) {
        return false;
      }
      state[0].setState(state[1]);
      return true;
    }

    render() {
      return this.state.subComponent;
    }

    handleLoggedIn() {
      var subComponent = <LandingPage application = {this} />;
      this.setState(
        {
          isLoggedIn: true,
          subComponent: subComponent
        });
    }
  }
  return ApplicationPage;
});
