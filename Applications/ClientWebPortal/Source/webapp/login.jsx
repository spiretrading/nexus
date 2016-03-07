require(['react', 'react-dom', 'ReactRouter.min', 'jquery', 'TimerMixin'],
  function(React, ReactDOM, ReactRouter, $, TimerMixin) {
    var Router = ReactRouter.Router;
    var Route = ReactRouter.Route;
    var Link = ReactRouter.Link;
    var submitStyle = {};
    var logoStyle = {};
    var submitted;
    var LoginPage = React.createClass({
      mixins: [TimerMixin],
      render: function() {
        return (
          <div className="login_page">
            <img ref="logo" id="logo" src={this.state.logo_src}
            alt="Spire Trading Logo" style={logoStyle} />
            <form ref="login_form" id="login_form" onSubmit={this.handleSubmit}>
              <input 
                autoFocus 
                className="login_input" 
                id="login_username" 
                type="text" 
                name="login_username" 
                placeholder="Username" 
                ref="login_username"
                value={this.state.username}
                onChange={this.handleUsernameChange} /><br/>
              <input 
                className="login_input" 
                id="login_password" 
                type="password"
                name="login_password" 
                placeholder="Password"
                ref="login_password"
                value={this.state.password}
                onChange={this.handlePasswordChange} /><br/>
              <input 
                id="login_submit" 
                type="submit"
                ref="login_submit"
                value="Login" 
                style={submitStyle} />
              <ul className="error_messages">
                {this.errorMessages}
              </ul>
            </form>
          </div>);
      },
      getInitialState: function() {
        submitted = false;
        return {
          username: ''
          , password: ''
          , logo_src: 'img/spire_white.png'
          , errorMessages: ''
        };
      },
      componentDidMount: function () {
        console.log("submitted: " + submitted);
      },
      componentWillUpdate: function () {
        console.log("component is getting updated!");
        if (submitted) {
          
        } else {
          submitStyle = {
          opacity: 1
          };
        }
      },
      componentWillUnmount: function() {
        this.serverRequest.abort();
      },
      handleSubmit: function(e) {
        e.preventDefault();
        submitStyle = {
          opacity: 0.9
          };
        logoStyle = {
          opacity: 1
        }
        console.log("Handle submit is working!");
        submitted = true;
        console.log("submitted after handleSubmit func: " + submitted);
        var logoSource = this.state.logo_src;
        console.log("logoSource state: " + logoSource);
        var newLogoSource = 'img/spire_animation_white_gradient.gif';
        var logoImage = this.refs['logo'];
        console.log("logoImage: " + logoImage);
        var timeoutID = window.setTimeout( function () {
          //animation
          console.log("image src before: " + this.state.logo_src);
          this.setState({logo_src : 'img/spire_animation_white_gradient.gif'});
          console.log("image src after: " + this.state.logo_src);}.bind(this),2000);
        var submitted_username = this.state.username.trim();
        var submitted_password = this.state.password.trim();
        if (!submitted_username) {
          console.log("No username or password");
          return;
        }
        this.setState({username: ''});
        this.setState({password: ''});
        console.log("Entered Username is: "+ submitted_username);
        console.log("Entered Password is: "+ submitted_password);
        $.ajax(
          {
            url: this.props.url,
            dataType: 'json',
            method: 'POST',
            data: JSON.stringify(
              {
                username: submitted_username,
                password: submitted_password
              })
          }).done(
            function(data, status, xhr) {
              console.log("done:  " + data);
              console.log("Request was successful");
              console.log("Response data: " + JSON.stringify(data) +
                " Status: " + status + " xhr: "+ xhr);
              $.ajax(
              {
                url: "/api/service_locator/logout",
                method: 'POST'
              }).done(
                function () {
                  console.log("Logged out and back to login page!");
                  submitted = false;
                  console.log("submitted: " + submitted);
                  submitStyle = {
                    opacity: initial
                  };
                  window.clearTimeout(timeoutID);
                  window.location.href = "/index.html"
                }.bind(this));
            }.bind(this)).fail(
            function(data, xhr, status, err) {
              submitted = false;
              console.log("submitted: " + submitted);
              submitStyle = {
                opacity: inherit
              };
              console.log("style of submitted: " + this.refs.login_submit.style);
              console.log("Request failed! ERROR Section");
              console.log("fail data:  " + data);
              console.log("Response data: " + JSON.stringify(data) +
                " Status: " + status + " xhr: "+ xhr);
              this.setState({errorMessages: '<li>' + 'the username and password you entered don\’t match' + '</li>'});
              window.clearTimeout(timeoutID);
            }.bind(this));
      },
      handleUsernameChange: function (e) {
        this.setState({username: e.target.value});
      },
      handlePasswordChange: function (e) {
        this.setState({password: e.target.value});
      },
    });

    ReactDOM.render(<LoginPage url="/api/service_locator/login" />,
      document.getElementById("container"));
});
