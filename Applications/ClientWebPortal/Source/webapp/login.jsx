var submitStyle = {};
var LoginPage = React.createClass({
  render: function() {
    return (
      <div className="login_page">
        <img ref="logo" id="logo" src={this.state.logo_src}
        alt="Spire Trading Logo"/>
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
            onChange={this.handlePasswordChange} /><br/>
          <input 
            id="login_submit" 
            type="submit" 
            value="Login" 
            ref="login_submit"
            style={submitStyle} />
          <ul className="error_messages"></ul>
        </form>
      </div>);
  },
  getInitialState: function() {
    return {username: '', password: '', submitted: false,
      logo_src: 'img/spire_white.png'};
  },
  componentDidMount: function () {

  },
  componentWillUpdate: function () {
    if (this.state.submitted) {
      submitStyle = {
      opacity: 0.9
      };
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
    console.log("Handle submit is working!");
    this.setState.submitted = true;
    e.preventDefault();
    this.refs.login_form.setTimeout( function () {
      //animation
      console.log("image src before: " + this.state.logo_src);
      this.setState.logo_src = 'img/spire_animation_white_gradient.gif';
      console.log("image src after: " + this.state.logo_src);},2000);
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
              this.setState.submitted = false;
              window.location.href = "/index.html"
            }.bind(this));
        }.bind(this)).fail(
        function(data, xhr, status, err) {
          this.setState.submitted = false;
          console.log("this.state.submitted: " + this.state.submitted);
          console.log("Request failed! ERROR Section");
          console.log("fail data:  " + data);
          console.log("Response data: " + JSON.stringify(data) +
            " Status: " + status + " xhr: "+ xhr);
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
