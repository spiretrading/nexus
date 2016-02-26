    var LoginPage = React.createClass({
            render: function() {
                return (<div className="login_page">
                        <img id="logo" src="img/spire_white.png" alt="Spire Trading Logo"/>
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
              return {username: ''};
            },
            componentDidMount: function(){
              
            },
            handleSubmit: function(e) {
              e.preventDefault();
              var author = this.state.username.trim();
              if (!username) {
                return;
              }
              // TODO: send request to the server
              this.setState({username: ''});
            },
            onMouseEnter: function (e) {

            },
            onMouseLeave: function (e) {

            },
            handleUsernameChange: function (e) {
              this.setState({username: e.target.value});
            },
            disappear: function () {

            },
            sendLoginRequest: function () {
              console.log(event); // => nullified object.
              console.log(event.type); // => "click"
              var eventType = event.type; // => "click"

              setTimeout(function() {
                console.log(event.type); // => null
                console.log(eventType); // => "click"
              }, 0);

              this.setState({clickEvent: event}); // Won't work. this.state.clickEvent will only contain null values.
              this.setState({eventType: event.type});

              var xhttp;
              var data = JSON.stringify(formToJson(document.getElementById("login-form")));
              if (window.XMLHttpRequest) {
                // code for modern browsers
                xhttp = new XMLHttpRequest();
                } else {
                // code for IE6, IE5
                xhttp = new ActiveXObject("Microsoft.XMLHTTP");
              }
              xhttp.onreadystatechange = function() {
                if (xhttp.readyState == 4 && xhttp.status == 200) {
                  //document.getElementById("demo").innerHTML = xhttp.responseText;

                }
              };
              xhttp.open("POST", "/api/service_locator/login", false);
              xhttp.withCredentials = true;
              xhttp.send(data);
              window.location.href = '/dashboard';
            } 
        });
        ReactDOM.render(<LoginPage />, document.getElementById("container"));