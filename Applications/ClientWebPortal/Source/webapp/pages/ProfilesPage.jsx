define(function(require) {
  var CSS = require('css!app/css/landing_page');
  var Radium = require('radium');
  var React = require('react');
  var ToggleButton = require('app/components/ToggleButton');

  class ProfileButton extends React.Component {
    render() {
      var buttonStyle = {
        backgroundColor: '#2ca1db',
        border: 'none',
        color: 'white',
        width: this.props.width,
        height: '32px',
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

  const TRADER = 1;
  const MANAGER = 2;
  const ADMINISTRATOR = 4;
  const SERVICE_PROVIDER = 8;

  function makeGroupEntry(directoryEntry, roles) {
    return (
      {
        directoryEntry: directoryEntry,
        roles: roles,
      });
  }

  class GroupTable extends React.Component {
    constructor(props) {
      super(props);
      this.state = {
        entries: []
      };
    }

    render() {
      var groupTableStyle = {
        color: 'black',
        fontSize: '20'
      };
      return (
        <table style = {groupTableStyle}>
          <tbody>
          {
            this.state.entries.map(
              function(entry) {
                return (
                  <tr key = {entry.directoryEntry.id}>
                    <td>
                      {entry.directoryEntry.name}
                    </td>
                  </tr>);
              }.bind(this))
          }
          </tbody>
        </table>);
    }

    componentDidMount() {
      var client = this.props.client;
      var tradingGroupPromise = client.administrationClient.loadTradingGroup(
        this.props.group);
      tradingGroupPromise.then(
        function(tradingGroup) {
          var managers = tradingGroup.managers.map(makeGroupEntry, MANAGER);
          var traders = tradingGroup.traders.map(makeGroupEntry, TRADER);
          var entries = managers.concat(traders);
          entries.sort(
            function(left, right) {
              return left.directoryEntry.name < right.directoryEntry.name;
            });
          this.setState({entries: entries});
        }.bind(this));
    }
  }

  function makeProfileEntry(entry) {
    return (
      {
        entry: entry,
        isExpanded: false
      });
  }

  class ProfileTable extends React.Component {
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
      var profileTableStyle = {
        color: 'black',
        fontSize: '20'
      };
      return (
        <table style = {profileTableStyle}>
          <tbody>
          {
            this.state.entries.map(
              function(entry) {
                var components = [];
                components.push(
                  <tr key = {entry.entry.id}>
                    <td>
                      <ToggleButton
                        baseImage = "/img/arrow_side_dark.svg"
                        baseHighlightedImage = "/img/arrow_side_light.svg"
                        toggledImage = "/img/arrow_down_dark.svg"
                        toggledHighlightedImage = "/img/arrow_down_light.svg"
                        width = "20px"
                        height = "20px"
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
                if(entry.isExpanded) {
                  components.push(
                    <tr>
                      <td colSpan = "2">
                        <GroupTable
                          client = {this.props.client}
                          group = {entry.entry} />
                      </td>
                    </tr>);
                }
                return components;
              }.bind(this))
          }
          </tbody>
        </table>);
    }

    handleTradingGroupToggled(isExpanded, tradingGroup) {
      tradingGroup.isExpanded = isExpanded;
      this.forceUpdate();
    }
  }

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
      var filterBarInputStyle = {
        border: '1px solid #bdc0c2',
        width: '100%',
        height: '28px',
        background: 'url("/img/magnifying_glass.svg") no-repeat 8px 6px',
        backgroundSize: '22px 22px',
        backgroundPosition: 'right 4px center'
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
            <ProfileTable
              ref = "profileTable"
              client = {this.props.application.client} />
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
          this.refs.profileTable.updateEntries(tradingGroups);
        }.bind(this));
    }
  }
  return ProfilesPage;
});
