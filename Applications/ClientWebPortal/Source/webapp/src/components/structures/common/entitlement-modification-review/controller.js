import {
  AdministrationClient
} from 'spire-client'
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import HashMap from 'hashmap';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.requiredDataLoaded = false;

    // this.loadModificationRequests = this.loadModificationRequests.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then(responses => {
      this.componentModel.changes = this.diffEntitlements(responses[0], responses[1].entitlements);
      this.componentModel.entitlements = definitionsService.getEntitlements();
      this.requiredDataLoaded = true;
      this.view.update(this.componentModel);
    });
  }

  isRequiredDataLoaded() {
    return this.requiredDataLoaded;
  }

  /** @private */
  getRequiredData() {
    let loadAccountEntitlements = this.adminClient.loadAccountEntitlements(
      this.componentModel.changeAccount
    );
    let loadEntitlementModification = this.adminClient.loadEntitlementModification(
      this.componentModel.modificationId
    );

    return Promise.all([
      loadAccountEntitlements,
      loadEntitlementModification
    ]);
  }

  /** @private */
  diffEntitlements(current, modification) {
    let combined = new HashMap();
    for (let i=0; i<current.length; i++) {
      combined.set(current[i].id, {
        current: current[i]
      });
    }
    for (let i=0; i<modification.length; i++) {
      if (!combined.has(modification[i].id)) {
        combined.set(modification[i].id, {});
      }
      combined.get(modification[i].id).modification = modification[i];
    }

    let changes = [];
    let ids = combined.keys();
    for (let i=0; i<ids.length; i++) {
      let result = combined.get(ids[i]);
      if (result.current != null && result.modification == null) {
        // remove case
        changes.push({
          type: 'remove',
          directoryEntry: result.current
        });
      } else if (result.current == null && result.modification != null) {
        // add case
        changes.push({
          type: 'add',
          directoryEntry: result.modification
        });
      }
    }
  }
}

export default Controller;
