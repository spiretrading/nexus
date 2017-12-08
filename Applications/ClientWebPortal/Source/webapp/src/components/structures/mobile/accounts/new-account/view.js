import './style.scss';
import React from 'react';
import PersonalDetails from 'components/reusables/mobile/new-personal-details';
import AccountPicture from 'components/reusables/common/account-picture';
import PrimaryButton from 'components/reusables/common/primary-button';
import TextArea from 'components/reusables/common/text-area';
import userService from 'services/user';
import CommonView from 'components/structures/common/profile/account/common-view';

class MobileView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let content;
    let personalDetailsModel,
      accountPictureModel,
      createButtonModel,
      userNotesModel,
      createButton;
    if (!this.controller.isModelEmpty()) {
      personalDetailsModel = JSON.parse(JSON.stringify(this.componentModel));
      personalDetailsModel.isReadOnly = !this.componentModel.isAdmin;

      accountPictureModel = {
        picture: this.componentModel.photoId,
        showLabel: false,
        isReadOnly: !userService.isAdmin()
      };

      userNotesModel = {
        text: this.componentModel.userNotes,
        isReadOnly: !userService.isAdmin()
      };

      createButtonModel = {
        label: 'Create Account'
      };

      let onAccountPictureChange = this.controller.onAccountPictureChange.bind(this.controller);
      let onPersonalDetailsChange = this.controller.onPersonalDetailsChange.bind(this.controller);
      let onUserNotesChange = this.controller.onUserNotesChange.bind(this.controller);
      let createAccount = this.controller.createAccount.bind(this.controller);

      if (userService.isAdmin()) {
        createButton =
          <div className="row create-button-wrapper">
            <PrimaryButton className="create-button" model={createButtonModel} onClick={createAccount}/>
            <div className="create-message"></div>
          </div>
      }

      content =
        <div>
          <div className="top-page-header row">
            Create Account
          </div>
          <div className="account-picture-wrapper row">
            <AccountPicture model={accountPictureModel} onPictureChange={onAccountPictureChange}/>
          </div>
          <div className="personal-details-wrapper">
            <PersonalDetails model={personalDetailsModel} onChange={onPersonalDetailsChange}/>
          </div>
          <div className="user-notes-header row">
            User Notes
          </div>
          <div className="row">
            <TextArea model={userNotesModel} onChange={onUserNotesChange}/>
          </div>
          {createButton}
        </div>
    }

    return (
      <div id="new-account-container">
        {content}
      </div>
    );
  }
}

export default MobileView;
