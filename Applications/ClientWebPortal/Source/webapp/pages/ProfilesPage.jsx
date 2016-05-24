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

  class ArrowButton extends React.Component {
    constructor(props) {
      super(props);
      this.state = {
        isExpanded: this.props.isExpanded
      };
      this.handleClick = this.handleClick.bind(this);
    }

    render() {
      var direction = function() {
        if(this.state.isExpanded) {
          return 'down';
        } else {
          return 'side';
        }
      }.bind(this)();
      var buttonStyle = {
        backgroundColor: 'transparent',
        border: 'none',
        width: '20px',
        height: '20px',
        backgroundImage: 'url("/img/arrow_' + direction + '_dark.svg")',
        backgroundSize: '20px 20px',
        ':hover': {
          backgroundImage: 'url("/img/arrow_' + direction + '_light.svg")'
        }
      };
      return (
        <button
          type = "button"
          style = {buttonStyle}
          onClick = {this.handleClick} />);
    }

    handleClick() {
      this.setState({isExpanded: !this.state.isExpanded},
        function() {
          this.props.onClick(this.state.isExpanded);
        });
    }
  }

  ArrowButton.propTypes =
    {
      isExpanded: React.PropTypes.bool,
      onClick: React.PropTypes.func
    };
  ArrowButton.defaultProps =
    {
      isExpanded: false,
      onClick: function(isExpanded) {}
    };

  ArrowButton = Radium(ArrowButton);

  function makeProfileEntry(entry) {
    return (
      {
        entry: entry,
        isExpanded: false
      });
  }

  class ProfilesPage extends React.Component {
    constructor() {
      super();
      this.state = {
        entries: []
      };
      this.handleTradingGroupToggled = this.handleTradingGroupToggled.bind(
        this);
    }

    updateEntries(entries) {
      var profileEntries = entries.map(makeProfileEntry);
      this.setState({entries: profileEntries});
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
        height: '28px',
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
      var profileTableStyle = {
        color: 'black',
        fontSize: '20'
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
            <div style = {{height: "15px"}} />
            <table style = {profileTableStyle}>
              <tbody>
              {
                this.state.entries.map(
                  function(entry) {
                    return (
                      <tr key = {entry.entry.id}>
                        <td>
                          <ArrowButton
                            isExpanded = {entry.isExpanded}
                            onClick = {
                              function(isExpanded) {
                                return this.handleTradingGroupToggled(
                                  isExpanded, entry);
                              }.bind(this)
                            }
                          />
                        </td>
                        <td>
                          {entry.entry.name}
                        </td>
                      </tr>);
                  }.bind(this))
              }
              </tbody>
            </table>
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
          this.updateEntries(tradingGroups);
        }.bind(this));
    }

    handleTradingGroupToggled(isExpanded, tradingGroup) {
      tradingGroup.isExpanded = isExpanded;
    }
  }
  return ProfilesPage;
});
