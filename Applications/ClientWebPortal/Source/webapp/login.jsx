    var LoginPage = React.createClass({
            render: function() {
                return (<div className="login_page">
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
                            onMouseLeave={this.incOpacity}
                            onClick={this.handleSubmit} />
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
              // TODO: send request to the server
              this.setState({username: ''});
              this.setState({password: ''});
              console.log("Entered Username is: "+ submitted_username);
              console.log("Entered Password is: "+ submitted_password);
              $.ajax({
                url: this.props.url,
                dataType: 'json',
                type: 'POST',
                data: {username: submitted_username, password: submitted_password},
                success: function(data) {
                  console.log("Request was successful");
                  ReactDOM.render(<Dashboard />,
                  document.getElementById('container'));
                  this.setState({data: data});
                }.bind(this),
                error: function(xhr, status, err) {
                  console.log("Request failed!");
                  //this.setState({data: {username, password}});
                  console.log(status);
                }.bind(this)
              });    
            },
            onMouseEnter: function (e) {

            },
            onMouseLeave: function (e) {

            },
            handleUsernameChange: function (e) {
              this.setState({username: e.target.value});
            },
            handlePasswordChange: function (e) {
              this.setState({password: e.target.value});
            },
            disappear: function () {

            }, 
        });
    //Dashboard Page
    var Dashboard = React.createClass({
      render: function() {
        return (
          <div>
            <h3> Welcome to Spire</h3>
            <div id="dashboard_container" />
            ReactDOM.render(
              <TimerExample start={Date.now()} />,
              document.getElementById('dashboard_container')
            );
          </div>
        );
      }
    });
    //Timer
    var Timer = React.createClass({

      getInitialState: function(){

          // This is called before our render function. The object that is 
          // returned is assigned to this.state, so we can use it later.

          return { elapsed: 0 };
      },

      componentDidMount: function(){

          // componentDidMount is called by react when the component 
          // has been rendered on the page. We can set the interval here:

          this.timer = setInterval(this.tick, 50);
      },

      componentWillUnmount: function(){

          // This method is called immediately before the component is removed
          // from the page and destroyed. We can clear the interval here:

          clearInterval(this.timer);
      },

      tick: function(){

          // This function is called every 50 ms. It updates the 
          // elapsed counter. Calling setState causes the component to be re-rendered

          this.setState({elapsed: new Date() - this.props.start});
      },

      render: function() {
          
          var elapsed = Math.round(this.state.elapsed / 100);

          // This will give a number with one digit after the decimal dot (xx.x):
          var seconds = (elapsed / 10).toFixed(1);    

          // Although we return an entire <p> element, react will smartly update
          // only the changed parts, which contain the seconds variable.

          return <p>Timer was started <b>{seconds} seconds</b> ago.</p>;
      }
    });
        ReactDOM.render(<LoginPage url="/api/service_locator/login" />, document.getElementById("container"));