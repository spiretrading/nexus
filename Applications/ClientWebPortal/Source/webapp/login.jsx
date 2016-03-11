define(function(require) {
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
      var submitted_username = this.refs.loginUsername.state.value.trim();
      var submitted_password = this.refs.loginPassword.state.value.trim();
      if(submitted_username == '') {
        return;
      }
      this.setState({submitted: true});
      var jqxhr = $.ajax(
        {
          url: this.props.url,
          dataType: 'json',
          method: 'POST',
          data: JSON.stringify(
            {
              username: submitted_username,
              password: submitted_password
            })
        }
      ).done(
        function(data, status, xhr) {
          $.ajax(
            {
              url: '/api/service_locator/logout',
              method: 'POST'
            }
          ).done(
            function () {
              this.setState({ submitted : false });
              window.location.href = '/index.html'
            }.bind(this));
        }.bind(this)
      ).fail(
        function(data, xhr, status, err) {
          this.setState({ submitted : false });
          if(status == 'abort') {
            this.setState({ errorMessages : 'unable to connect, check your connection' });
          } else {
            this.setState({ errorMessages : 'the username and password you entered don\’t match'});
          }
        }.bind(this));
      window.setTimeout(jqxhr.abort, this.props.timeout);
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
      /** The amount of time (in milliseconds) before the login attempt should
       *  abort.
       */
       timeout: React.PropTypes.number,

       /** The URL used to authenticate the login. */
       url: React.PropTypes.string
   };
   LoginPage.defaultProps =
    {
      timeout: 5000,
      url: "/api/service_locator/login"
    };
    ReactDOM.render(<LoginPage />, document.getElementById('container'));
  }
);
