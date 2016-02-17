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
