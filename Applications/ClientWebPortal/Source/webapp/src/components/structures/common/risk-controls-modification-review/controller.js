import {
  AdministrationClient,
  DirectoryEntry,
  MessageBody,
  Message
} from 'spire-client'
import preloaderTimer from 'utils/preloader-timer';
import {browserHistory} from 'react-router';
import moment from 'moment';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();

    this.onOkClick = this.onOkClick.bind(this);
    this.approveRequest = this.approveRequest.bind(this);
    this.rejectRequest = this.rejectRequest.bind(this);
    this.onCommentsInput = this.onCommentsInput.bind(this);
    this.approveRequest = this.approveRequest.bind(this);
    this.rejectRequest = this.rejectRequest.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let loadRiskModification = this.adminClient.loadRiskModification(this.componentModel.modificationId);
    let loadRiskParamsResolve;
    let loadRiskParamsPromise = new Promise(resolve => {
      loadRiskParamsResolve = resolve;
    });
    let loadAccountModificationRequest = this.adminClient.loadAccountModificationRequest(this.componentModel.modificationId)
      .then(request => {
        let account = request.account.clone();
        this.adminClient.loadRiskParameters(account)
          .then(riskParams => {
            loadRiskParamsResolve(riskParams);
          });

        return request;
      });
    let loadComments = this.adminClient.loadMessageIds(this.componentModel.modificationId)
      .then(messageIds => {
        let loadMessages = [];
        for (let i=0; i<messageIds.length; i++) {
          loadMessages.push(
            this.adminClient.loadMessage(messageIds[i])
          );
        }
        return Promise.all(loadMessages);
      });
    let loadModificationStatus = this.adminClient.loadAccountModificationStatus(this.componentModel.modificationId);
    return Promise.all([
      loadRiskModification,
      loadAccountModificationRequest,
      loadRiskParamsPromise,
      loadComments,
      loadModificationStatus
    ]);
  }

  componentDidMount() {
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then(results => {
      this.componentModel.riskModification = results[0];
      this.componentModel.account = results[1].account;
      this.componentModel.requesterAccount = results[1].submissionAccount;
      this.componentModel.originalRiskParams = results[2];
      this.componentModel.comments = results[3];
      this.componentModel.requestStatus = results[4].status;
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    return this.componentModel.riskModification != null;
  }

  onOkClick() {
    browserHistory.goBack();
  }

  onCommentsInput(comments) {
    this.componentModel.newComments = comments;
  }

  approveRequest() {
    let messageBody = new MessageBody('text/plain', this.componentModel.newComments || "");
    let timestamp = moment.utc().format('YYYYMMDDTHHmmss');
    this.adminClient.approveAccountModificationRequest(
      this.componentModel.modificationId,
      new Message(-1, DirectoryEntry.DEFAULT, timestamp, [messageBody])
    ).then(() => {
      browserHistory.push('/modification-request-history');
    });
  }

  rejectRequest() {
    let messageBody = new MessageBody('text/plain', this.componentModel.newComments || "");
    let timestamp = moment.utc().format('YYYYMMDDTHHmmss');
    this.adminClient.rejectAccountModificationRequest(
      this.componentModel.modificationId,
      new Message(-1, DirectoryEntry.DEFAULT, timestamp, [messageBody])
    ).then(() => {
      browserHistory.push('/modification-request-history');
    });
  }
}

export default Controller;
