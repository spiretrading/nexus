import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import AccModRequest from 'components/reusables/desktop/account-modification-request';
import EntitlementPanel from 'components/reusables/common/entitlement-panel';
import TextArea from 'components/reusables/common/text-area';
import PrimaryButton from 'components/reusables/common/primary-button';
import { AccountModificationRequestStatus } from 'spire-client';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#entitlement-modification-review-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  render() {
    let content;

    if (this.controller.isRequiredDataLoaded()) {
      let statusMessage;
      if (this.componentModel.requestStatus == AccountModificationRequestStatus.SCHEDULED ||
          this.componentModel.requestStatus == AccountModificationRequestStatus.GRANTED) {
        statusMessage = <div className="status-message-wrapper approved">
                          <span className="icon-request-approved"/> APPROVED
                        </div>
      } else if (this.componentModel.requestStatus == AccountModificationRequestStatus.REJECTED) {
        statusMessage = <div className="status-message-wrapper rejected">
                          <span className="icon-request-rejected"/> REJECTED
                        </div>
      }

      let entitlementPanels = [];
      for (let i=0; i<this.componentModel.changes.length; i++) {
        let model = {
          entitlement: this.componentModel.entitlements.get(this.componentModel.changes[i].directoryEntry.id),
          isSelected: this.componentModel.changes[i].type == 'add',
          isReadOnly: true,
          isGreen: this.componentModel.changes[i].type == 'add',
          isRed: this.componentModel.changes[i].type == 'remove'
        };
        entitlementPanels.push(
          <div key={i} className="entitlement-panel-wrapper">
            <EntitlementPanel model={model}/>
          </div>
        );
      }

      let comments = [];
      for (let i=0; i<this.componentModel.comments.length; i++) {
        let author = this.componentModel.comments[i].account.name;
        let messageBodies = [];
        for (let j=0; j<this.componentModel.comments[i].bodies.length; j++) {
          let messageText = this.componentModel.comments[i].bodies[j].message;
          let message = <div key={j}>
                          {messageText}
                        </div>
          messageBodies.push(message);
        }
        comments.push(
          <div key={i} className="comment-wrapper">
            <div className="author">{author}</div>
            <div className="bodies-wrapper">
              {messageBodies}
            </div>
          </div>
        );
      }

      let commentsInputModel = {
        text: "",
        isReadOnly: false,
        placeHolder: 'Leave comment here...'
      };

      let onCommentsInput = this.controller.onCommentsInput;
      let footer;
      if (this.componentModel.requestStatus == AccountModificationRequestStatus.PENDING && this.componentModel.isAuthority) {
        footer =  <div className="footer-wrapper">
                    <div className="new-comments-wrapper">
                      <TextArea model={commentsInputModel} onChange={onCommentsInput}/>
                    </div>
                    <div className="buttons-wrapper">
                      <div className="approve-wrapper">
                        <PrimaryButton className="approve-button" model={{ label: 'Approve Request' }} onClick={this.controller.approveRequest}/>
                      </div>
                      <div className="reject-wrapper">
                        <PrimaryButton className="reject-button" model={{ label: 'Reject Request' }} onClick={this.controller.rejectRequest}/>
                      </div>
                    </div>
                  </div>
      } else {
        footer =  <div className="footer-wrapper confirm">
                    <PrimaryButton className="confirm-button" model={{ label: 'OK' }} onClick={this.controller.navigateBack}/>
                  </div>
      }

      content =
        <div>
          <div className="page-top-header row">
            Entitlement Request
          </div>
          {statusMessage}
          <div className="info">
            <div className="details-row">
              Account: <span>{this.componentModel.changeAccount.name}</span><br/>
              Requested by: <span>{this.componentModel.requesterAccount.name}</span><br/>
              Request ID: <span>{this.componentModel.modificationId}</span>
            </div>
          </div>
          <div className="entitlement-panels-wrapper">
            {entitlementPanels}
          </div>
          <hr className="footer-divider"/>
          <div className="comments-wrapper">
            {comments}
          </div>
          {footer}
        </div>
    }

    return (
      <div id="entitlement-modification-review-container">
        {content}
      </div>
    );
  }
}

export default View;
