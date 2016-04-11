define(function(require) {
  var React = require('react');

  class AccountStatsTable extends React.Component {
    render() {
      var style = {
        height: '150px'
      };
      return (
        <div style = {style}>
          Account Stats<br />
          Accounts: 100<br />
          Groups: 123<br />
          Administrators: 5<br />
          Managers: 10<br />
        </div>);
    }
  };

  class DirectoryEntryTableEntry extends React.Component {
    render() {
      return (
        <tr key = {this.props.key}>
          <td>
            <button type = "button">+</button>{this.props.entry}
          </td>
          <td>Group</td>
        </tr>);
    }
  }

  class DirectoryEntryTable extends React.Component {
    render() {
      var style = {
        width: '100%'
      };
      var idStyle = {};
      return (
        <table style = {style}>
          <thead>
            <tr>
              <th style = {idStyle}>ID</th>
              <th>Roles</th>
            </tr>
          </thead>
          <tbody>
            {this.props.entries.map(
              function(entry, key) {
                return (
                  <DirectoryEntryTableEntry
                    entry = {entry}
                    key = {key} />
                );
              })
            }
          </tbody>
        </table>
      );
    }
  };

  class ProfilesPage extends React.Component {
    constructor() {
      super();
      this.state = {
        entries: []
      };
    }

    render() {
      var containerStyle = {
        width: '100%',
        display: 'flex',
        flexDirection: 'column',
        flexWrap: 'nowrap'
      };
      var accountStatsTableStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      var toolbarStyle = {
        width: '100%',
        flexGrow: 0,
        flexBasis: 'content',
        display: 'flex',
        flexDirection: 'row'
      };
      var filterBarStyle = {
        flexGrow: 1
      };
      var filterBarInputStyle = {
        width: '100%'
      };
      var filterButtonStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      var toolbarSpacerStyle = {
        flexGrow: 1
      };
      var addUserButtonStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      var createGroupButtonStyle = {
        flexGrow: 0,
        flexBasis: 'content'
      };
      var tableStyle = {
        flexGrow: 1
      };
      return (
        <div style = {containerStyle}>
          <div style = {accountStatsTableStyle}>
            <AccountStatsTable />
          </div>
          <div style = {toolbarStyle}>
            <div style = {filterBarStyle}>
              <input
                style = {filterBarInputStyle}
                type = "text"
                value = "Filter" />
            </div>
            <div style = {filterButtonStyle}>
              <input
                type = "button"
                value = "OK" />
            </div>
            <div style = {toolbarSpacerStyle} />
            <div style = {addUserButtonStyle}>
              <input
                type = "button"
                value = "Add User" />
            </div>
            <div style = {createGroupButtonStyle}>
              <input
                type = "button"
                value = "Create Group" />
            </div>
          </div>
          <div style = {tableStyle}>
            <DirectoryEntryTable
              entries = {this.state.entries} />
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
