import {AdministrationClient, ServiceLocatorClient, AccountIdentity, DirectoryEntry} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.serviceLocatorClient = new ServiceLocatorClient();
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
    let loadManagedTradingGroups = this.adminClient.loadManagedTradingGroups.apply(this.adminClient, [directoryEntry]);

    return Promise.all([
      loadManagedTradingGroups
    ]);
  }

  componentDidMount() {
    let directoryEntry = userService.getDirectoryEntry();
    this.componentModel = {
      directoryEntry: directoryEntry
    };

    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.managedTradingGroups = responses[0];
      this.componentModel.roles = {
        isAdmin: false,
        isManager: false,
        isService: false,
        isTrader: false
      };

      $.extend(true, this.componentModel, {
        addressLineOne: '',
        addressLineThree: '',
        addressLineTwo: '',
        city: '',
        country: 0,
        email: '',
        firstName: '',
        lastName: '',
        pictures: '',
        province: '',
        userName: '',
        userNotes: ''
      });
      this.componentModel.isAdmin = userService.isAdmin();
      this.componentModel.groups = [];
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

  onPersonalDetailsChange(newPersonalDetails) {
    $.extend(true, this.componentModel, newPersonalDetails);
  }

  createAccount() {
    let model = this.componentModel;
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

    this.serviceLocatorClient.createAccount(
      this.componentModel.userName,
      this.componentModel.groups[0] || null,
      accountIdentity,
      this.componentModel.roles
    )
      .then(navigateToNewAccountProfile)
      .catch(this.view.showCreateFailMessage);

    function navigateToNewAccountProfile(directoryEntry) {
      browserHistory.push('/profile-account/' +
        directoryEntry.type +
        '/' + directoryEntry.id +
        '/' + directoryEntry.name);
    }
  }
}

export default Controller;
