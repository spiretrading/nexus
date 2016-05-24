define(function(require) {
  var CSS = require('css!app/css/landing_page');
  var Radium = require('radium');
  var React = require('react');

  class ProfileButton extends React.Component {
    render() {
      var buttonStyle = {
        backgroundColor: '#2ca1db',
        border: 'none',
        color: 'white',
        width: this.props.width,
        height: '28px',
        fontFamily: 'Roboto',
        fontStyle: 'light',
        ':hover': {
          backgroundColor: '#aadaf0'
        }
      };
      return (
        <button
            type = "button"
            style = {buttonStyle}
            onClick = {this.props.onClick}>
          {this.props.itemName}
        </button>);
    }
  }

  ProfileButton = Radium(ProfileButton);

  class ProfilesPage extends React.Component {
    constructor() {
      super();
      this.state = {
        entries: []
      };
    }

    render() {
      var bodyStyle = {
        width: '100%',
        display: 'flex',
        flexDirection: 'column',
        flexWrap: 'nowrap',
        alignItems: 'center'
      };
      var containerStyle = {
        width: '960px',
        display: 'flex',
        flexDirection: 'column',
        flexWrap: 'nowrap'
      };
      var toolbarStyle = {
        flexGrow: 0,
        height: '28px',
        display: 'flex',
        flexDirection: 'row'
      };
      var filterBarStyle = {
        width: '625px',
        height: '28px'
      };
      var filterBarInputStyle = {
        border: '1px solid #bdc0c2',
        width: '100%',
        height: '28px',
        background: 'url("/img/magnifying_glass.svg") no-repeat 8px 6px',
        backgroundSize: '22px 22px',
        backgroundPosition: 'right 4px center'
      };
      var filterButtonStyle = {
        backgroundColor: 'white',
        border: '1px solid #bdc0c2',
        borderLeft: 'none',
        color: 'white',
        width: '28px',
        height: '28px'
      };
      var addUserButtonStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      var createGroupButtonStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      return (
        <div style = {bodyStyle}>
          <div style = {containerStyle}>
            <div style = {toolbarStyle}>
              <input
                style = {filterBarInputStyle}
                type = "text" />
              <div style = {{width: "15px"}} />
              <ProfileButton
                itemName = "New Account"
                width = "120px" />
              <div style = {{width: "8px"}} />
              <ProfileButton
                itemName = "New Group"
                width = "120px" />
            </div>
          </div>
        </div>);
    }

    componentDidMount() {
      var client = this.props.application.client;
      var tradingGroupsPromise =
        client.administrationClient.loadManagedTradingGroups(
        client.loadCurrentAccount());
      tradingGroupsPromise.then(
        function(tradingGroups) {
          this.setState({entries : tradingGroups});
        }.bind(this));
    }
  }
  return ProfilesPage;
});
