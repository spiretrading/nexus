requirejs.config({
  baseUrl: 'third-party',
});

// Start loading the main app file. Put all of
// your application logic in there.
// requirejs(['app/main']);

require(['react', 'react-dom', 'ReactRouter.min'],
  function(React, ReactDOM, ReactRouter) {
    var Router = ReactRouter.Router;
    var Route = ReactRouter.Route;
    var Link = ReactRouter.Link;
    console.log(Router);
    var MyInput = React.createClass({
      getInitialState: function() {
        return {
          value: ""
        };
      },
      
      handleChange: function(e) {
        this.setState({
          value: e.target.value
        });
      },
      
      render: function() {
        return (
          <div>
            {this.props.name}
            <input value={this.state.value} onChange={this.handleChange} />
          </div>);
      }
    });

    var LoginButton = React.createClass({
      handleClick: function(event) {
        event.preventDefault();
        window.location.href = "http://www.google.com";
      },

      render: function() {
        return (
          <button onClick={this.handleClick}>Login</button>
        );
      }
    });

    ReactDOM.render(
      <div>
        <MyInput id="username" name="Username" /><br />
        <MyInput id="password" name="Password" /><br />
        <button>Login</button>
      </div>,
      document.getElementById('container')
    );
  });
