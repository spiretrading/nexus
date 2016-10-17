import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import EntitlementPanel from 'components/reusables/common/entitlement-panel';
import PrimaryButton from 'components/reusables/common/primary-button';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#entitlements-container').fadeIn({
      duration: Config.FADE_DURATION
    });
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

  render() {
    let userInfoNavModel, entitlements;
    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    let onEntitlementSelected = this.controller.onEntitlementSelected.bind(this.controller);
    let onEntitlementDeselected = this.controller.onEntitlementDeselected.bind(this.controller);

    if (this.controller.isModelInitialized.apply(this.controller)) {
      userInfoNavModel = {
        userName: this.componentModel.directoryEntry.name,
        roles: this.componentModel.roles
      };

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
          isSelected: isSelected,
          isAdmin: this.componentModel.isAdmin
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

    let onSave = this.controller.save.bind(this.controller);

    let hr, saveButton;
    if (this.componentModel.isAdmin) {
      hr = <hr />;
      saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>;
    }

    return (
      <div id="entitlements-container" className={className}>
        <div className="user-info-nav-wrapper">
          <UserInfoNav model={userInfoNavModel}/>
        </div>
        {entitlements}
        {hr}
        {saveButton}
        <div className="save-message-wrapper">
          <div className="save-message"></div>
        </div>
      </div>
    );
  }
}

export default View;
