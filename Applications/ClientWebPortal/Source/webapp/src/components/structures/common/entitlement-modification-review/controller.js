import {
  AdministrationClient,
  DirectoryEntry,
  MessageBody,
  Message
} from 'spire-client'
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import HashMap from 'hashmap';
import moment from 'moment';
import {browserHistory} from 'react-router';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.requiredDataLoaded = false;

    this.onCommentsInput = this.onCommentsInput.bind(this);
    this.approveRequest = this.approveRequest.bind(this);
    this.rejectRequest = this.rejectRequest.bind(this);
    this.navigateBack = this.navigateBack.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then(responses => {
      this.componentModel.changes = this.diffEntitlements(responses[0], responses[1].entitlements);

      let entitlements = definitionsService.getEntitlements();
      this.componentModel.entitlements = new HashMap();
      for (let i=0; i<entitlements.length; i++) {
        this.componentModel.entitlements.set(
          entitlements[i].group_entry.id,
          entitlements[i]
        );
      }

      this.componentModel.comments = responses[2];
      this.requiredDataLoaded = true;
      this.view.update(this.componentModel);
    });
  }

  isRequiredDataLoaded() {
    return this.requiredDataLoaded;
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

  navigateBack() {
    browserHistory.goBack();
  }

  /** @private */
  getRequiredData() {
    let loadAccountEntitlements = this.adminClient.loadAccountEntitlements(
      this.componentModel.changeAccount
    );
    let loadEntitlementModification = this.adminClient.loadEntitlementModification(
      this.componentModel.modificationId
    );
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
      loadAccountEntitlements,
      loadEntitlementModification,
      loadComments
    ]);
  }

  /** @private */
  diffEntitlements(current, modification) {
    let combined = new HashMap();
    for (let i=0; i<current.length; i++) {
      combined.set(current[i].id, {
        current: current[i]
      });
    }
    for (let i=0; i<modification.length; i++) {
      if (!combined.has(modification[i].id)) {
        combined.set(modification[i].id, {});
      }
      combined.get(modification[i].id).modification = modification[i];
    }

    let changes = [];
    let ids = combined.keys();
    for (let i=0; i<ids.length; i++) {
      let result = combined.get(ids[i]);
      if (result.current != null && result.modification == null) {
        // remove case
        changes.push({
          type: 'remove',
          directoryEntry: result.current
        });
      } else if (result.current == null && result.modification != null) {
        // add case
        changes.push({
          type: 'add',
          directoryEntry: result.modification
        });
      }
    }

    return changes;
  }
}

export default Controller;
