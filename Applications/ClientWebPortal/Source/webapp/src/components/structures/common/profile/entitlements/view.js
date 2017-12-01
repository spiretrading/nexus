import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import EntitlementPanel from 'components/reusables/common/entitlement-panel';
import PrimaryButton from 'components/reusables/common/primary-button';
import modal from 'utils/modal';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
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
    modal.show($('#req-submitted-modal'));
  }

  render() {
    let entitlements;
    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    let onEntitlementSelected = this.controller.onEntitlementSelected.bind(this.controller);
    let onEntitlementDeselected = this.controller.onEntitlementDeselected.bind(this.controller);

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
    }

    let saveBtnModel = {
      label: 'Save Changes'
    };

    let submitRequestBtnModel = {
      label: 'Submit Request'
    };

    let onSave = this.controller.save.bind(this.controller);
    let onSubmitRequest = this.controller.submitRequest.bind(this.controller);

    let hr, saveButton;
    if (this.componentModel.isAdmin) {
      hr = <hr />;
      saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>;
    } else {
      hr = <hr />;
      saveButton = <PrimaryButton className="save-button" model={submitRequestBtnModel} onClick={onSubmitRequest}/>;
    }

    return (
      <div id="entitlements-container" className={className}>
        {entitlements}
        {hr}
        {saveButton}
        <div className="save-message-wrapper">
          <div className="save-message"></div>
        </div>

        <div id="req-submitted-modal" className="modal fade" tabIndex="-1" role="dialog">
          <div className="modal-dialog" role="document">
            <div className="modal-content">
              <div className="modal-header">
                Change Picture
              </div>
              <div className="modal-body change-picture-wrapper">
                Hi this is some dummy message
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

export default View;
