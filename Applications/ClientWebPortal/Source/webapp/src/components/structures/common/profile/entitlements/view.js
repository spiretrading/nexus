import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import EntitlementPanel from 'components/reusables/common/entitlement-panel';
import PrimaryButton from 'components/reusables/common/primary-button';
import modal from 'utils/modal';
import TextArea from 'components/reusables/common/text-area';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onSubmitOkClick = this.onSubmitOkClick.bind(this);

    this.commentsModel = {
      text: "",
      isReadOnly: false,
      placeHolder: 'Leave comment here...'
    };
  }

  componentDidUpdate() {
    $('#entitlements-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  hideSaveMessage() {
    $('#entitlements-container .save-message').stop().fadeOut();
  }

  showSaveSuccessMessage() {
    let $saveMessage = $('#entitlements-container .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Saved')
          .removeClass('red')
          .addClass('purple')
          .fadeIn();
      });
  }

  showSaveFailMessage() {
    let $saveMessage = $('#entitlements-container .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Failed')
          .removeClass('purple')
          .addClass('red')
          .fadeIn();
      });
  }

  showRequestSubmittedMessage(requestId) {
    $('#entitlements-container .modal .request-id').text(requestId);
    modal.show($('#entitlements-container .modal'));
  }

  render() {
    let entitlements;
    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    let onEntitlementSelected = this.controller.onEntitlementSelected.bind(this.controller);
    let onEntitlementDeselected = this.controller.onEntitlementDeselected.bind(this.controller);

    let saveBtnModel, submitRequestBtnModel, saveButton, comments, submitOkBtnModel, hr;
    if (this.controller.isModelInitialized()) {
      entitlements = [];
      for (let i=0; i<this.componentModel.entitlements.length; i++) {
        let entitlement = this.componentModel.entitlements[i];
        let id = entitlement.group_entry.id;
        let isSelected = false;
        for (let j=0; j<this.componentModel.accountEntitlements.length; j++) {
          if (this.componentModel.accountEntitlements[j].id === id) {
            isSelected = true;
            break;
          }
        }
        let model = {
          entitlement: entitlement,
          isSelected: isSelected
        };
        entitlements.push(
          <EntitlementPanel key={i}
                            model={model}
                            onSelected={onEntitlementSelected}
                            onDeselected={onEntitlementDeselected}/>
        );
      }

      saveBtnModel = {
        label: 'Save Changes'
      };

      submitRequestBtnModel = {
        label: 'Submit Request'
      };

      let onSave = this.controller.save.bind(this.controller);
      let onSubmitRequest = this.controller.submitRequest.bind(this.controller);

      if (this.componentModel.isAdmin) {
        saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>;
      } else {
        let onCommentsChange = this.controller.onCommentsChange.bind(this.controller);
        comments =  <div className="comments-wrapper">
                      <TextArea model={this.commentsModel} onChange={onCommentsChange} />
                    </div>;
        saveButton = <PrimaryButton className="save-button" model={submitRequestBtnModel} onClick={onSubmitRequest}/>;
      }
      hr = <hr />;

      submitOkBtnModel = {
        label: 'OK'
      };
    }

    return (
      <div id="entitlements-container" className={className}>
        {entitlements}
        {hr}
        {comments}
        {saveButton}
        <div className="save-message-wrapper">
          <div className="save-message"></div>
        </div>

        <div id="req-submitted-modal" className="modal fade" tabIndex="-1" role="dialog">
          <div className="modal-dialog" role="document">
            <div className="modal-content">
              <div className="modal-header">
                <span className="icon-request-approved"/> Request Submitted
              </div>
              <div className="modal-body">
                <div>Request ID</div>
                <div className="request-id"></div>
                <div>You will be notified when your<br/>request is processed.</div>
              </div>
              <PrimaryButton className="submit-ok-btn" model={submitOkBtnModel} onClick={this.onSubmitOkClick}/>
            </div>
          </div>
        </div>
      </div>
    );
  }

  /** @private */
  onSubmitOkClick() {
    modal.hide($('#req-submitted-modal'));
  }
}

export default View;
