import './style.scss';
import React from 'react';
import PrimaryButton from 'components/reusables/common/primary-button';
import CommonView from 'components/structures/common/profile/risk-controls/common-view';
import definitionsService from 'services/definitions';
import userService from 'services/user';
import { AccountModificationRequestStatus } from 'spire-client';
import TextArea from 'components/reusables/common/text-area';

class View extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  getChangeClass(isNoChange) {
    let className = '';
  }

  render() {
    let content;
    if (this.controller.isModelInitialized()) {
      let original = this.componentModel.originalRiskParams;
      let change = this.componentModel.riskModification.parameters;
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

      let footer;
      if ((userService.isManager() && this.componentModel.status == AccountModificationRequestStatus.PENDING) || 
        (userService.isAdmin() && (this.componentModel.status == AccountModificationRequestStatus.PENDING || this.componentModel.status == AccountModificationRequestStatus.REVIEWED)))
      {
        let commentsInputModel = {
          text: "",
          isReadOnly: false,
          placeHolder: 'Leave comment here...'
        };
        let onCommentsInput = this.controller.onCommentsInput;

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
        footer =  <div className="button-wrapper">
                    <PrimaryButton className="ok-button" model={{ label: 'OK' }} onClick={this.controller.onOkClick}/>
                  </div>
      }

      content = <div className="content-wrapper">
                  <div className="title-wrapper">
                    Risk Controls Request
                  </div>
                  <div className="account-details">
                    <div className="account">
                      Account: {this.componentModel.account.name}
                    </div>
                    <div className="requested">
                      Requested by: {this.componentModel.requesterAccount.name}
                    </div>
                    <div className="request-id">
                      Request ID: {this.componentModel.modificationId}
                    </div>
                  </div>
                  <div className="change-details">
                    <table>
                      <thead>
                        <tr>
                          <td>Parameters</td>
                          <td className="purple">Current</td>
                          <td className="purple">Change</td>
                        </tr>
                      </thead>
                      <tbody>
                        <tr>
                          <td>Currency</td>
                          <td>{definitionsService.getCurrencyCode(original.currencyId)}</td>
                          <td className={change.currencyId.equals(original.currencyId) ? '' : 'changed' }>
                            {definitionsService.getCurrencyCode(change.currencyId)}
                          </td>
                        </tr>
                        <tr>
                          <td>Buying Power</td>
                          <td>{original.buyingPower.toString()}</td>
                          <td className={change.buyingPower.equals(original.buyingPower) ? '' : 'changed' }>
                            {change.buyingPower.toString()}
                          </td>
                        </tr>
                        <tr>
                          <td>Net Loss</td>
                          <td>{original.netLoss.toString()}</td>
                          <td className={change.netLoss.equals(original.netLoss) ? '' : 'changed' }>
                            {change.netLoss.toString()}
                          </td>
                        </tr>
                        <tr>
                          <td>Transition Time</td>
                          <td>{original.transitionTime}</td>
                          <td className={change.transitionTime == original.transitionTime ? '' : 'changed' }>
                            {change.transitionTime}
                          </td>
                        </tr>
                      </tbody>
                    </table>
                  </div>
                  <hr />
                  <div className="comments-wrapper">
                    {comments}
                  </div>
                  {footer}
                </div>
    }

    return (
      <div id="risk-controls-modification-review-container" className="container-fixed-width">
        {content}
      </div>
    );
  }
}

export default View;
