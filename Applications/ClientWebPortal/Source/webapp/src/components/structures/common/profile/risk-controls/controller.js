import profileContext from 'components/structures/common/profile/context';
import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
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
    let loadAccountRoles = adminClient.loadRiskParameters.apply(adminClient, [directoryEntry]);

    return Promise.all([
      loadAccountRoles
    ]);
  }

  componentDidMount() {
    let context = profileContext.get();
    let directoryEntry = context.directoryEntry;
    this.componentModel = {
      directoryEntry: directoryEntry
    };
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, Config.WHOLE_PAGE_PRELOADER_WIDTH, Config.WHOLE_PAGE_PRELOADER_HEIGHT).then((responses) => {
      let riskParameters = responses[0];
      this.componentModel.riskParameters = riskParameters;
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.roles = context.roles;
      this.componentModel.userName = context.userName;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isModelEmpty() {
    let model = cloneObject(this.componentModel);
    delete model.componentId;
    return $.isEmptyObject(model);
  }

  onCurrencyChange(newCurrencyNumber) {
    this.componentModel.riskParameters.currency = newCurrencyNumber;
    this.view.update(this.componentModel);
  }

  onNetLossChange(newAmount) {
    this.componentModel.riskParameters.netLoss = newAmount;
  }

  onBuyingPowerChange(newAmount) {
    this.componentModel.riskParameters.buyingPower = newAmount;
  }

  onTransitionTimeChange(newTime) {
    this.componentModel.riskParameters.transitionTime = newTime;
  }

  save() {
    let riskParameters = this.componentModel.riskParameters;
    let context = profileContext.get();
    let directoryEntry = context.directoryEntry;
    adminClient.storeRiskParameters.apply(adminClient, [directoryEntry, riskParameters])
      .then(onSaved.bind(this))
      .catch(onFailed.bind(this));

    function onSaved() {
      this.view.showSavedMessage('Saved');
    }

    function onFailed() {
      this.view.showSaveFailedMessage('Failed');
    }
  }
}

export default Controller;