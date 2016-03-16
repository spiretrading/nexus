define(function(require) {
  var CSS = require('css!app/css/login');
  var React = require('react');
  var ReactDOM = require('react-dom');
  var $ = require('jquery');
  var AnimatedImage = require('app/components/AnimatedImage');
  var PlaceholderAlignedInput = require(
    'app/components/PlaceholderAlignedInput');

  /** A React Component displaying Spire's login page. */
  class LoginPage extends React.Component {

    /**
     * Constructs a LoginPage.
     */
    constructor() {
      super();
      this.state =
        {
          submitted: false,
          errorMessages: ''
        };
      this.handleSubmit = this.handleSubmit.bind(this);
    }

    /** @private */
    handleSubmit(event) {
      event.preventDefault();
      var username = this.refs.loginUsername.state.value.trim();
      var password = this.refs.loginPassword.state.value.trim();
      if(username.length == 0) {
        return;
      }
      this.setState({submitted: true});
      this.props.client.login(username, password).then(
        function() {
          this.setState({ submitted : false });
          this.props.success();
        }.bind(this),
        function(reason) {
          this.setState(
            {
              submitted: false,
              errorMessages: reason
            });
        }.bind(this));
    }

    render() {
      return (
        <div className = "login-page">
          <AnimatedImage
            alt = "Spire Trading Logo"
            id = "logo"
            ref = "logo"
            initialImage = "img/spire_white.png"
            animatedImage = "img/spire_loading_animation.gif"
            isPlaying = {this.state.submitted}
          />
          <form
            ref = "loginForm"
            id = "login-form"
            onSubmit = {this.handleSubmit}>
            <PlaceholderAlignedInput
              autoFocus
              className = "login-input"
              id = "login-username"
              ref = "loginUsername"
              type = "text"
              name = "login_username"
              placeholder = "Username"
            />
            <br />
            <PlaceholderAlignedInput
              className = "login-input"
              id = "login-password"
              ref = "loginPassword"
              type = "password"
              name = "login_password"
              placeholder = "Password"
            />
            <br />
            <input
              className = {
                function() {
                  if(this.state.submitted) {
                    return "inactive";
                  } else {
                    return '';
                  }
                }.bind(this)()
              }
              id = "login-submit"
              ref = "loginSubmit"
              type = "submit"
              value = "Login"
            />
            <p className = "error-messages">{this.state.errorMessages}</p>
          </form>
        </div>);
    }
  }
  LoginPage.propTypes =
    {

      /** The SpireWebClient to use. */
      client: React.PropTypes.object.isRequired,

      /** The function to call on success. */
      success: React.PropTypes.func.isRequired,
    };
  return LoginPage;
});
