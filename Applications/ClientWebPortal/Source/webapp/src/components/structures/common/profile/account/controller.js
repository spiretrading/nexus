import routeParameters from 'utils/route-parameters';
import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
import serviceLocatorClient from 'utils/spire-clients/service-locator';
import context from 'components/structures/common/profile/context';
import ResultCode from 'utils/spire-clients/service-locator/result-codes';
import userService from 'services/user';

class Controller {
  constructor(componentModel) {
    this.componentModel = cloneObject(componentModel);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRoles = adminClient.loadAccountRoles.apply(adminClient, [directoryEntry]);
    let loadAccountProfile = adminClient.loadAccountProfile.apply(adminClient, [directoryEntry]);

    return Promise.all([
      loadAccountRoles,
      loadAccountProfile
    ]);
  }

  isModelEmpty() {
    let model = cloneObject(this.componentModel);
    delete model.componentId;
    return $.isEmptyObject(model);
  }

  componentDidMount() {
    let directoryEntry = routeParameters.get();
    this.componentModel = {
      directoryEntry: directoryEntry
    };

    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, Config.WHOLE_PAGE_PRELOADER_WIDTH, Config.WHOLE_PAGE_PRELOADER_HEIGHT).then((responses) => {
      this.componentModel.roles = responses[0];
      $.extend(true, this.componentModel, responses[1]);
      context.set({
        directoryEntry: directoryEntry,
        roles: responses[0],
        userName: responses[1].userName
      });
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  onAccountPictureChange(newPictureData) {
    this.componentModel.photoId = newPictureData;
    this.view.update(this.componentModel);
  }

  onUserNotesChange(newNotes) {
    this.componentModel.userNotes = newNotes;
  }

  onPasswordUpdate(newPassword) {
    serviceLocatorClient.storePassword(this.componentModel.directoryEntry, newPassword)
      .then(onResponse.bind(this))
      .catch(this.view.showSavePasswordFailMessage);

    function onResponse(response) {
      if (response.resultCode === ResultCode.SUCCESS) {
        this.view.showSavePasswordSuccess();
      } else {
        this.view.showSavePasswordFailMessage();
      }
    }
  }

  onPersonalDetailsChange(newPersonalDetails) {
    $.extend(true, this.componentModel, newPersonalDetails);
  }

  save() {
    let accountIdentity = cloneObject(this.componentModel);
    let directoryEntry = accountIdentity.directoryEntry;
    delete accountIdentity.roles;
    delete accountIdentity.directoryEntry;
    adminClient.storeAccountIdentity.apply(adminClient, [directoryEntry, accountIdentity])
      .then(this.view.showSavePersonalDetailsSuccessMessage)
      .catch(this.view.showSavePersonalDetailsFailMessage);
  }
}

export default Controller;
