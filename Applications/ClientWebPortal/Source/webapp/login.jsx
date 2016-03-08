require(['react', 'react-dom', 'ReactRouter.min', 'jquery'],
  function(React, ReactDOM, ReactRouter, $) {
    var Router = ReactRouter.Router;
    var Route = ReactRouter.Route;
    var Link = ReactRouter.Link;

    var SpireLogo = React.createClass({
      getInitialState: function () {
        return {
          is_playing : false,
        };
      },
      play: function() {
        this.setState({is_playing : true});
      },
      stop: function () {
        this.setState({is_playing : false});
      },
      render: function () {
        return (
          <img id="logo"
              src={
                function() {
                  if(this.state.is_playing) {
                    return this.props.animated_image;
                  } else {
                    return this.props.initial_image;
                  }
                }.bind(this)()
              }
            alt="Spire Trading Logo" />
          );
      }
    });
    var LoginPage = React.createClass({
      render: function() {
        return (
          <div className="login_page">
            <SpireLogo ref = "logo" initial_image = {'img/spire_white.svg'} 
              animated_image = {'img/spire_loading_animation.gif'} />
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
                className={this.state.submitted ? "inactive" : ""}
                type="submit"
                ref="login_submit"
                value="Login" />
              <p className="error_messages"> {this.state.errorMessages} </p>
            </form>
          </div>);
      },
      getInitialState: function() {
        return {
          username: ''
          , password: ''
          , submitted : false
          , errorMessages: ''
        };
      },
      componentDidMount: function () {
        console.log("submitted: " + this.state.submitted);
        $(".login_input").focus(function(){
          console.log("I have focuson");
        });
      },
      componentWillUpdate: function () {
      },
      componentWillUnmount: function() {
      },
      handleSubmit: function(e) {
        e.preventDefault();
        this.setState({ submitted: true });
        console.log("submitted after handleSubmit func: " + this.state.submitted);
        console.log("Handle submit is working!");
        var submitted_username = this.state.username.trim();
        var submitted_password = this.state.password.trim();
        if (!submitted_username) {
          console.log("No username or password");
          return;
        }
        this.setState({username: ''});
        this.setState({password: ''});
        console.log("Entered Username is: " + submitted_username);
        console.log("Entered Password is: " + submitted_password);
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
                  this.setState({ submitted : false });
                  console.log("submitted: " + this.state.submitted);
                  window.location.href = "/index.html"
                }.bind(this));
            }.bind(this)).fail(
            function(data, xhr, status, err) {
              console.log("Request failed! ERROR Section");
              this.setState({ submitted : false });
              console.log("submitted: " + this.state.submitted);
              if ( status == "abort") {
                this.setState({ errorMessages : 'unable to connect, check your connection' });
                console.log("errorMsg: " + this.state.errorMessages);
              } else {
                this.setState({ errorMessages : 'the username and password you entered don\’t match'});
                console.log("errorMsg: " + this.state.errorMessages);
              }
              console.log("fail data:  " + data);
              console.log("Response data: " + JSON.stringify(data) +
                " Status: " + status + " xhr: " + xhr);
            }.bind(this)).always(
              function () {
                this.refs.logo.stop();
                window.clearTimeout(timeoutID);
              }.bind(this)
            );
        this.refs.logo.play();
        var timeoutID = window.setTimeout( function () {
          jqxhr.abort();
          }.bind(this),5000);
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
