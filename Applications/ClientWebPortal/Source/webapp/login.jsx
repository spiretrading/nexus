var LoginPage = React.createClass({
  render: function() {
    return (
      <div className="login_page">
        <img id="logo" src="img/spire_white.png"
        alt="Spire Trading Logo"/>
        <form id="login_form" onSubmit={this.handleSubmit}>
          <input 
            autoFocus 
            className="login_input" 
            id="login_username" 
            type="text" 
            name="login_username" 
            placeholder="Username" 
            ref="login_username"
            value={this.state.username}
            onChange={this.handleUsernameChange}
            onClick={this.disappear}/><br/>
          <input 
            className="login_input" 
            id="login_password" 
            type="password"
            name="login_password" 
            placeholder="Password"
            ref="login_password" 
            onChange={this.handlePasswordChange}
            onClick={this.disappear} /><br/>
          <input 
            id="login_submit" 
            type="submit" 
            value="Login" 
            ref="login_submit"
            onMouseEnter={this.decOpacity} 
            onMouseLeave={this.incOpacity} />
          <ul className="error_messages"></ul>
        </form>
      </div>);
  },
  getInitialState: function() {
    return {username: '', password: ''};
  },
  componentDidMount: function(){
    
  },
  handleSubmit: function(e) {
    console.log("Handle submit is working!");
    e.preventDefault();
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
          window.location.href = "/dashboard";
        }.bind(this)).fail(
        function(data, xhr, status, err) {
          console.log("Request failed! ERROR Section");
          console.log("fail data:  " + data);
          console.log("Response data: " + JSON.stringify(data) +
            " Status: " + status + " xhr: "+ xhr);
        }.bind(this));
  },
  onMouseEnter: function (e) {},
  onMouseLeave: function (e) {},
  handleUsernameChange: function (e) {
    this.setState({username: e.target.value});
  },
  handlePasswordChange: function (e) {
    this.setState({password: e.target.value});
  },
  disappear: function () {}, 
});

var Dashboard = React.createClass({
  render: function() {
    return (
      <div>
        <h3> Welcome to Spire</h3>
        <div id="dashboard_container" />
        <Timer start={Date.now()} />
      </div>
    );
  },

  componentDidMount: function() {
    this.serverRequest = $.get(this.props.source, function (result) {
      var lastGist = result[0];
      console.log("result: " + result);
      console.log("result[0]: " + lastGist);
    }.bind(this));
  },

  componentWillUnmount: function() {
    this.serverRequest.abort();
  }
});

var Timer = React.createClass({
  getInitialState: function(){
    return { elapsed: 0 };
  },
  componentDidMount: function(){
    this.timer = setInterval(this.tick, 1);
  },
  componentWillUnmount: function(){
    clearInterval(this.timer);
  },
  tick: function(){
    this.setState({elapsed: new Date() - this.props.start});
  },
  render: function() {
    var elapsed = Math.round(this.state.elapsed / 100);
    var seconds = (elapsed / 10).toFixed(1);    
    return <p>Timer was started <b>{seconds} seconds</b> ago.</p>;
  }
});

ReactDOM.render(<LoginPage url="/api/service_locator/login" />,
  document.getElementById("container"));
