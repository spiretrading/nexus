import {AdministrationClient, DirectoryEntry} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import HashMap from 'hashmap';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();

    this.navigateToMemberProfile = this.navigateToMemberProfile.bind(this);
    this.isModelInitialized = this.isModelInitialized.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  navigateToMemberProfile(memberId) {
    let member;
    for (let i=0; i<this.componentModel.members.length; i++) {
      if (memberId == this.componentModel.members[i].id) {
        member = this.componentModel.members[i];
        break;
      }
    }

    browserHistory.push('/profile-account/' +
      member.type +
      '/' + member.id +
      '/' + member.name);
  }

  /** @private */
  getRequiredData() {
    let traders;
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRoles = this.adminClient.loadTradingGroup(directoryEntry)
      .then((accounts) => {
        traders = accounts.traders;
        let requestedRoles = new HashMap();
        let loadRolesPromises = [];
        for (let i=0; i<traders.length; i++) {
          let traderDirectoryEntry = traders[i];
          traderDirectoryEntry = new DirectoryEntry(
            traderDirectoryEntry.id,
            traderDirectoryEntry.type,
            traderDirectoryEntry.name
          );
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

        for (let i=0; i<traders.length; i++) {
          traders[i].roles = rolesMap.get(traders[i].id);
        }

        return traders;
      });

    return Promise.all([
      loadAccountRoles
    ]);
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, Config.WHOLE_PAGE_PRELOADER_WIDTH, Config.WHOLE_PAGE_PRELOADER_HEIGHT).then((responses) => {
      this.componentModel.groupName = directoryEntry.name;
      this.componentModel.members = responses[0];
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }
}

export default Controller;
