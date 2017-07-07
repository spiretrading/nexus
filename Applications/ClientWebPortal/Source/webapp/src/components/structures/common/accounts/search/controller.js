import {AdministrationClient, ServiceLocatorClient, DirectoryEntry} from 'spire-client';
import userService from 'services/user';
import preloaderTimer from 'utils/preloader-timer';
import HashMap from 'hashmap';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.accountDirectoryEntries = new HashMap();
    this.adminClient = new AdministrationClient();
    this.serviceLocatorClient = new ServiceLocatorClient();

    this.createGroup = this.createGroup.bind(this);
    this.findGroupDirectoryEntry = this.findGroupDirectoryEntry.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  isModelEmpty() {
    let model = clone(this.componentModel);
    delete model.componentId;
    return $.isEmptyObject(model);
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadManagedTradingGroups = this.adminClient.loadManagedTradingGroups(directoryEntry);

    return Promise.all([
      loadManagedTradingGroups
    ]);
  }

  componentDidMount() {
    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.groupedAccounts = responses[0];
      this.componentModel.groupedAccounts.sort((a,b) => {
        if (a.name.toLowerCase() < b.name.toLowerCase()) return -1;
        if (a.name.toLowerCase() > b.name.toLowerCase()) return 1;
        return 0;
      });
      for (let i=0; i<this.componentModel.groupedAccounts.length; i++) {
        this.componentModel.groupedAccounts[i].isLoaded = false;
      }
      this.componentModel.searchString = '';
      this.view.update(this.componentModel);
    });
  }

  componentWillUnmount() {
    this.view.dispose();
  }

  search(searchString) {
    this.serviceLocatorClient.searchDirectoryEntry(searchString)
      .then((results) => {
        // results come in a mix of groups and individual traders in an array
        for (let i=0; i<results.length; i++) {
          this.accountDirectoryEntries.set(results[i].directory_entry.id, results[i].directory_entry);

          if (results[i].directory_entry.type == 0) {
            // in the case of individual trader
            let group = null;
            let groupId = results[i].group.id;
            // find existing group in the model
            for (let j=0; j<this.componentModel.groupedAccounts.length; j++) {
              if (this.componentModel.groupedAccounts[j].id == groupId) {
                group = this.componentModel.groupedAccounts[j];
                break;
              }
            }

            // initialize the group
            if (group.accounts == null) {
              group.accounts = {
                traders: []
              };
            }

            // add the trader in group if it does not exist
            let trader = results[i].directory_entry;
            trader.roles = results[i].roles;

            let doesTraderExist = false;
            for (let j=0; j<group.accounts.traders.length; j++) {
              if (group.accounts.traders[j].id == trader.id) {
                doesTraderExist = true;
              }
            }

            if (!doesTraderExist) {
              group.accounts.traders.push(trader);
            }
          }
        }

        this.componentModel.searchString = searchString;
        this.view.update(this.componentModel);
      });
  }

  navigateToTraderProfile(traderId) {
    let directoryEntry = this.accountDirectoryEntries.get(traderId);
    browserHistory.push('/profile-account/' +
      directoryEntry.type +
      '/' + directoryEntry.id +
      '/' + directoryEntry.name);
  }

  navigateToGroupProfile(groupId) {
    let directoryEntry = this.findGroupDirectoryEntry(groupId);
    browserHistory.push('/group-profile-account/' +
      directoryEntry.type +
      '/' + directoryEntry.id +
      '/' + directoryEntry.name);
  }

  navigateToNewAccount() {
    browserHistory.push('accounts-newAccount');
  }

  loadGroupAccounts(groupId) {
    let group = this.findGroupDirectoryEntry(groupId);
    let groupDirectoryEntry = new DirectoryEntry(
      group.id,
      group.type,
      group.name
    );
    this.adminClient.loadTradingGroup(groupDirectoryEntry)
      .then((accounts) => {
        group.isLoaded = true;
        group.accounts = accounts;
        let traders = accounts.traders;
        let requestedRoles = new HashMap();
        let loadRolesPromises = [];
        for (let i=0; i<traders.length; i++) {
          let traderDirectoryEntry = traders[i];
          traderDirectoryEntry = new DirectoryEntry(
            traderDirectoryEntry.id,
            traderDirectoryEntry.type,
            traderDirectoryEntry.name
          );
          this.accountDirectoryEntries.set(traderDirectoryEntry.id, traderDirectoryEntry);
          if (!requestedRoles.has(traderDirectoryEntry.id)) {
            loadRolesPromises.push(this.adminClient.loadAccountRoles(traderDirectoryEntry));
            requestedRoles.set(traderDirectoryEntry.id, true);
          }
        }
        return Promise.all(loadRolesPromises);
      })
      .then((roles) => {
        let rolesMap = new HashMap();
        for (let i=0; i<roles.length; i++) {
          rolesMap.set(roles[i].id, roles[i]);
        }

        let groupTraders = group.accounts.traders;
        for (let i=0; i<groupTraders.length; i++) {
          groupTraders[i].roles = rolesMap.get(groupTraders[i].id);
        }

        this.view.update(this.componentModel);
      });
  }

  /** @private */
  findGroupDirectoryEntry(groupId) {
    for (let i=0; i<this.componentModel.groupedAccounts.length; i++) {
      if (this.componentModel.groupedAccounts[i].id == groupId) {
        return this.componentModel.groupedAccounts[i];
      }
    }
  }

  createGroup(groupName) {
    this.serviceLocatorClient.createGroup(groupName)
      .then(this.view.closeCreateGroupModal.bind(this.view))
      .then(refreshSearchPage.bind(this));

    function refreshSearchPage() {
      this.getRequiredData()
        .then((responses) => {
          let groups = responses[0];
          for (let i=0; i<groups.length; i++) {
            let groupId = groups.id;
            if (!doesGroupExist.call(this, groupId)) {
              this.componentModel.groupedAccounts.push(groups[i]);
            }
          }
          this.componentModel.searchString = '';
          this.view.update(this.componentModel);
        });
    }

    function doesGroupExist(groupId) {
      let groupedAccounts = this.componentModel.groupedAccounts;
      for (let i=0; i<groupedAccounts.length; i++) {
        if (groupedAccounts[i].id === groupId) {
          return true;
        }
      }
      return false;
    }
  }
}

export default Controller;
