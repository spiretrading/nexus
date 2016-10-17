import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
import serviceLocatorClient from 'utils/spire-clients/service-locator';
import ResultCode from 'utils/spire-clients/service-locator/result-codes';
import userService from 'services/user';
import {browserHistory} from 'react-router/es6';

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

  getDirectoryEntry() {
    return this.componentModel.directoryEntry;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRoles = adminClient.loadAccountRoles.apply(adminClient, [directoryEntry]);
    let loadAccountIdentity = adminClient.loadAccountIdentity.apply(adminClient, [directoryEntry]);

    return Promise.all([
      loadAccountRoles,
      loadAccountIdentity
    ]);
  }

  /** @private */
  loadRequiredDataAndRender() {
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.roles = responses[0];
      $.extend(true, this.componentModel, responses[1]);
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = cloneObject(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  componentDidMount() {
    this.loadRequiredDataAndRender();
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
        this.view.resetInputs();
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

  reloadAcountProfile(type, id, name) {
    this.view.hideAccountProfile();
    this.loadRequiredDataAndRender();
  }

  setDirectoryEntry(type, id, name) {
    this.componentModel.directoryEntry = {
      id: id,
      name: name,
      type: type
    };
  }
}

export default Controller;
