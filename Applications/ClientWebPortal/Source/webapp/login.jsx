    var LoginPage = React.createClass({
            render: function() {
                return (<div className="loginPage">
                        <img id="logo" src="img/Spire_White.png" alt="Spire Trading Logo"/>
                        <form id="login-form">
                          <input className="login-input" id="username" type="text" name="Username" placeholder="Username"/><br/>
                          <input className="login-input" id="password" type="password" name="Password" placeholder="Password"/><br/>
                          <input id="login-submit" type="submit" value="Login" />
                          <ul className="errorMessages"></ul>
                        </form>
                      </div>);
            }
            /*,
            formToJson : function(selector) {
              var arr = $(selector).serializeArray();
              var obj = {};
              for(var a = 0; a < arr.length; a++) {
                obj[arr[a].name] = arr[a].value;
              }
              return obj;
            },
            sendLoginRequest: function () {
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
            } */
        });
        ReactDOM.render(<LoginPage />, document.getElementById("container"));