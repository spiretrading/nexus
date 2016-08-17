import routeParameters from 'utils/route-parameters';
import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
import serviceLocatorClient from 'utils/spire-clients/service-locator';

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
    let loadAccountRoles = adminClient.loadAccountRoles(directoryEntry);
    let loadAccountProfile = adminClient.loadAccountProfile(directoryEntry);

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
    this.componentModel = {
      directoryEntry: routeParameters.get()
    };
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, 49, 60).then((responses) => {
      this.componentModel.roles = responses[0];
      $.extend(true, this.componentModel, responses[1]);
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
      .then(this.view.showSavePasswordSuccess)
      .catch(this.view.showSavePasswordFailMessage);
  }

  onPersonalDetailsChange(newPersonalDetails) {
    $.extend(true, this.componentModel, newPersonalDetails);
  }

  save() {
    let accountIdentity = cloneObject(this.componentModel);
    let directoryEntry = accountIdentity.directoryEntry;
    delete accountIdentity.roles;
    delete accountIdentity.directoryEntry;
    adminClient.storeAccountIdentity(directoryEntry, accountIdentity)
      .then(this.view.showSavePersonalDetailsSuccessMessage)
      .catch(this.view.showSavePersonalDetailsFailMessage);
  }
}

export default Controller;
