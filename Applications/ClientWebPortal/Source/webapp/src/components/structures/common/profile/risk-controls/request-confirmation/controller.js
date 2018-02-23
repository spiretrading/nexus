import {
    AdministrationClient
  } from 'spire-client';
  import preloaderTimer from 'utils/preloader-timer';
  import {browserHistory} from 'react-router';
  
  class Controller {
    constructor(componentModel) {
      this.componentModel = clone(componentModel);
      this.adminClient = new AdministrationClient();

      this.onOkClick = this.onOkClick.bind(this);
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
      return Promise.all([
        loadRiskModification,
        loadAccountModificationRequest,
        loadRiskParamsPromise,
        loadComments
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
        this.view.update(this.componentModel);
      });
    }
  
    isModelInitialized() {
      return this.componentModel.riskModification != null;
    }

    onOkClick() {
      let account = this.componentModel.account;
      browserHistory.push(`/profile-riskControls/${account.type}/${account.id}/${account.name}`);
    }
  }
  
  export default Controller;
  