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

    render() {
      return (
        <div className = 'login_page'>
          <AnimatedImage
            alt = 'Spire Trading Logo'
            id = 'logo'
            ref = 'logo'
            initialImage = 'img/spire_white.png'
            animatedImage = 'img/spire_loading_animation.gif'
            isPlaying = {this.state.submitted}
          />
          <form
            ref = 'login_form'
            id = 'login_form'
            onSubmit = {this.handleSubmit}>
              <PlaceholderAlignedInput
                autoFocus
                className = 'login_input'
                id = 'login_username'
                ref = 'login_username'
                type = 'text'
                name = 'login_username'
                placeholder = 'Username'
              />
              <br />
              <PlaceholderAlignedInput
                className = 'login_input'
                id = 'login_password'
                ref = 'login_password'
                type = 'password'
                name = 'login_password'
                placeholder = 'Password'
              />
              <br />
              <input
                className = {
                  function() {
                    if(this.state.submitted) {
                      return 'inactive';
                    } else {
                      return '';
                    }
                  }.bind(this)()
                }
                id = 'login_submit'
                ref = 'login_submit'
                type = 'submit'
                value = 'Login'
              />
              <p className = 'error_messages'>{this.state.errorMessages}</p>
          </form>
        </div>);
    }

    /** @private */
    handleSubmit(event) {
      event.preventDefault();
      var submitted_username = this.refs.login_username.state.value.trim();
      var submitted_password = this.refs.login_password.state.value.trim();
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
        }.bind(this)
      );
      window.setTimeout(jqxhr.abort, this.props.timeout);
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
      url: '/api/service_locator/login'
    };

  ReactDOM.render(<LoginPage />, document.getElementById('container'));
});
