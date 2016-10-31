import {Admin, ServiceLocator} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import {ServiceLocatorResultCode} from 'spire-client';
import userService from 'services/user';

class Controller {
  constructor(componentModel) {
    this.componentModel = cloneObject(componentModel);
    this.adminClient = new Admin();
    this.serviceLocatorClient = new ServiceLocator();
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
    let loadAccountRoles = this.adminClient.loadAccountRoles.apply(this.adminClient, [directoryEntry]);
    let loadAccountIdentity = this.adminClient.loadAccountIdentity.apply(this.adminClient, [directoryEntry]);

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
    this.serviceLocatorClient.storePassword(this.componentModel.directoryEntry, newPassword)
      .then(onResponse.bind(this))
      .catch(this.view.showSavePasswordFailMessage);

    function onResponse(response) {
      if (response.resultCode === ServiceLocatorResultCode.SUCCESS) {
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
    this.adminClient.storeAccountIdentity.apply(this.adminClient, [directoryEntry, accountIdentity])
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
