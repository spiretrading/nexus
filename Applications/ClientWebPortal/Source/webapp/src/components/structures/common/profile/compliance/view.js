import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import PrimaryButton from 'components/reusables/common/primary-button';
import CompliancePanel from 'components/reusables/common/compliance-panel';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#compliance-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  showSavedMessage() {
    let $saveMessage = $('#compliance-container .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Saved')
          .removeClass('red')
          .addClass('purple')
          .fadeIn();
      });
  }

  showSaveFailedMessage(message) {
    let $saveMessage = $('#compliance-container .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text(message)
          .removeClass('purple')
          .addClass('red')
          .fadeIn();
      });
  }

  /** @private */
  getRuleEntryPanels() {
    let entryPanels = [];
    let entries = this.componentModel.complianceRuleEntries;
    let onRuleUpdate = this.controller.onRuleUpdate.bind(this.controller);
    for (let i=0; i<entries.length; i++) {
      let entry = entries[i];
      let model= {
        ruleEntryId: entry.id,
        schema: entry.schema,
        state: entry.state
      };
      entryPanels.push(
        <CompliancePanel key={i} model={model} onUpdate={onRuleUpdate}/>
      );
    }

    return entryPanels;
  }

  render() {
    let content;

    if (this.controller.isModelInitialized.apply(this.controller)) {
      let userInfoNavModel = {
        userName: this.componentModel.userName,
        roles: this.componentModel.roles
      };

      let complianceRuleEntries = this.getRuleEntryPanels();

      let onSave = this.controller.save.bind(this.controller);

      let saveBtnModel = {
        label: 'Save Changes'
      };
      let saveButton, horizontalDivider;
      if (this.componentModel.isAdmin) {
        saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>
        horizontalDivider = <hr/>
      }

      content =
        <div>
          <div className="row">
            <UserInfoNav model={userInfoNavModel}/>
          </div>
          {complianceRuleEntries}
          {horizontalDivider}
          {saveButton}
          <div className="save-message"></div>
        </div>
    }

    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    return (
      <div id="compliance-container" className={className}>
        {content}
      </div>
    );
  }
}

export default View;
