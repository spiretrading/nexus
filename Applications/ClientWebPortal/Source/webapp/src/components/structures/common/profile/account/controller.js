import {
  AdministrationClient,
  ServiceLocatorClient,
  ServiceLocatorResultCode,
  AccountIdentity,
  DirectoryEntry
} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();
    this.serviceLocatorClient = new ServiceLocatorClient();

    this.onPasswordUpdate = this.onPasswordUpdate.bind(this);
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
    let loadAccountRoles = this.adminClient.loadAccountRoles(directoryEntry);
    let loadAccountIdentity = this.adminClient.loadAccountIdentity(directoryEntry);

    return Promise.all([
      loadAccountRoles,
      loadAccountIdentity
    ]);
  }

  /** @private */
  loadRequiredDataAndRender() {
    let requiredDataFetchPromise = this.getRequiredData();
    let directoryEntry = this.componentModel.directoryEntry;

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      console.debug(responses);
      this.componentModel.roles = responses[0];
      $.extend(true, this.componentModel, responses[1]);
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  componentDidMount() {
    this.loadRequiredDataAndRender();
  }

  onAccountPictureChange(newPictureData) {
    this.view.hideSavePersonalDetailsMessage();
    this.componentModel.photoId = newPictureData;
    this.view.update(this.componentModel);
  }

  onUserNotesChange(newNotes) {
    this.view.hideSavePersonalDetailsMessage();
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
    this.view.hideSavePersonalDetailsMessage();
    $.extend(true, this.componentModel, newPersonalDetails);
  }

  save() {
    let model = this.componentModel;
    let directoryEntry = model.directoryEntry;
    let accountIdentity = new AccountIdentity(
      model.addressLineOne,
      model.addressLineTwo,
      model.addressLineThree,
      model.city,
      model.country,
      model.email,
      model.firstName,
      model.lastLoginTime,
      model.lastName,
      model.picture,
      model.province,
      model.registrationTime,
      model.userNotes
    );
    let saveRoles = this.adminClient.storeAccountRoles(directoryEntry, model.roles);
    this.adminClient.storeAccountIdentity(directoryEntry, accountIdentity)
      .then(saveRoles)
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
