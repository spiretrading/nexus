    var HelloWorld = React.createClass({
            render: function() {
                return 
                      <div>
                        <form action="demo_form.asp">
                          <input type="text" name="FirstName" placeholder="Username"><br>
                          <input type="text" name="LastName" placeholder="Password"><br>
                          <input type="submit" value="Login">
                        </form>
                      </div>;
            }
        });
        ReactDOM.render(<HelloWorld />, document.getElementById("container"));