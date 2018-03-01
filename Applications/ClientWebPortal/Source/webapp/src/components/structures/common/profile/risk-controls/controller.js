import {
  AdministrationClient,
  DirectoryEntry,
  CurrencyId,
  Money,
  MessageBody,
  Message,
  RiskModification
} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import moment from 'moment';
import {browserHistory} from 'react-router';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();

    this.onCurrencyChange = this.onCurrencyChange.bind(this);
    this.save = this.save.bind(this);
    this.onTransitionTimeChange = this.onTransitionTimeChange.bind(this);
    this.onCommentsChange = this.onCommentsChange.bind(this);
    this.submitModificationRequest = this.submitModificationRequest.bind(this);
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
    let loadAccountRiskParameters = this.adminClient.loadRiskParameters(directoryEntry);
    let loadAccountRoles = this.adminClient.loadAccountRoles(directoryEntry);

    return Promise.all([
      loadAccountRiskParameters,
      loadAccountRoles
    ]);
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.riskParameters = responses[0];
      this.originalRiskParameters = this.componentModel.riskParameters.clone();
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.roles = responses[1];
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);

      EventBus.publish(Event.Profile.VIEWING_CONTEXT_LOADED, {
        directoryEntry: this.componentModel.directoryEntry,
        roles: this.componentModel.roles
      });
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  onCurrencyChange(newCurrencyNumber) {
    this.view.hideSavedMessage();
    EventBus.publish(Event.Profile.RiskControls.CURRENCY_SELECTED);
    this.componentModel.riskParameters.currencyId = CurrencyId.fromData(newCurrencyNumber);
    this.view.update(this.componentModel);
  }

  onNetLossChange(newAmount) {
    this.view.hideSavedMessage();
    this.componentModel.riskParameters.netLoss = Money.fromNumber(newAmount);
  }

  onBuyingPowerChange(newAmount) {
    this.view.hideSavedMessage();
    this.componentModel.riskParameters.buyingPower = Money.fromNumber(newAmount);
  }

  onTransitionTimeChange(newTime) {
    this.view.hideSavedMessage();
    this.componentModel.riskParameters.transitionTime = newTime;
  }

  onCommentsChange(newComment) {
    this.componentModel.requestComments = newComment;
  }

  save() {
    if (this.componentModel.riskParameters.currency != 0) {
      let riskParameters = this.componentModel.riskParameters;
      let directoryEntry = this.componentModel.directoryEntry;
      this.adminClient.storeRiskParameters(directoryEntry, riskParameters)
        .then(onSaved.bind(this))
        .catch(onFailed.bind(this));
    } else {
      this.view.showSaveFailedMessage('Currency not selected');
    }

    function onSaved() {
      this.view.showSavedMessage('Saved');
    }

    function onFailed() {
      this.view.showSaveFailedMessage('Failed');
    }
  }

  submitModificationRequest() {
    if (this.componentModel.riskParameters.currency != 0) {
      let directoryEntry = this.componentModel.directoryEntry;
      let riskParameters = this.componentModel.riskParameters;
      let messageBody = new MessageBody('text/plain', this.componentModel.requestComments || "");
      let timestamp = moment.utc().format('YYYYMMDDTHHmmss');
      this.adminClient.submitRiskModificationRequest(
        directoryEntry,
        new RiskModification(riskParameters),
        new Message(-1, DirectoryEntry.DEFAULT, timestamp, [messageBody])
      )
        .then(accountModificationRequest => {
          browserHistory.push({
            pathname: '/risk-controls-request-confirmation/' + accountModificationRequest.id,
            state: {
              isFromApp: true
            }
          });
        });
    }
  }
}

export default Controller;
