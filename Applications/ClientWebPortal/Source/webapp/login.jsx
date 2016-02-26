    var LoginPage = React.createClass({
            render: function() {
                return (<div className="loginPage">
                        <img id="logo" src="img/spire_white.png" alt="Spire Trading Logo"/>
                        <form id="login-form">
                          <input className="login-input" id="username" type="text" name="Username" placeholder="Username"/><br/>
                          <input className="login-input" id="password" type="password" name="Password" placeholder="Password"/><br/>
                          <input id="login-submit" type="submit" value="Login" onClick={this.sendLoginRequest} onMouseEnter={this.decOpacity} onMouseLeave={this.incOpacity} />
                          <ul className="errorMessages"></ul>
                        </form>
                      </div>);
            },
            onMouseEnter: function (event) {

            },
            onMouseLeave: function (event) {

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
        console.log(); 