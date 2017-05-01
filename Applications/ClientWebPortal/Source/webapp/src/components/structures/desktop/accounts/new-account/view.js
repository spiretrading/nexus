import './style.scss';
import React from 'react';
import PersonalDetails from 'components/reusables/desktop/new-personal-details';
import AccountPicture from 'components/reusables/common/account-picture';
import PrimaryButton from 'components/reusables/common/primary-button';
import UserNotes from 'components/reusables/common/user-notes';
import CommonView from 'components/structures/common/accounts/new-account/common-view';

class DesktopView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let content;
    let personalDetailsModel,
      accountPictureModel,
      createButtonModel,
      userNotesModel,
      changePasswordButtonModel,
      createButton;

    if (!this.controller.isModelEmpty.apply(this.controller)) {
      personalDetailsModel = JSON.parse(JSON.stringify(this.componentModel));
      personalDetailsModel.isReadOnly = !this.componentModel.isAdmin;

      accountPictureModel = {
        picture: this.componentModel.photoId,
        showLabel: true,
        isReadOnly: !this.componentModel.isAdmin
      };

      userNotesModel = {
        userNotes: this.componentModel.userNotes,
        isReadOnly: !this.componentModel.isAdmin
      };

      createButtonModel = {
        label: 'Create Account'
      };

      let onAccountPictureChange = this.controller.onAccountPictureChange.bind(this.controller);
      let onPersonalDetailsChange = this.controller.onPersonalDetailsChange.bind(this.controller);
      let onUserNotesChange = this.controller.onUserNotesChange.bind(this.controller);
      let createAccount = this.controller.createAccount.bind(this.controller);

      if (this.componentModel.isAdmin) {
        createButton =
          <div className="row create-button-wrapper">
            <PrimaryButton className="create-button" model={createButtonModel} onClick={createAccount}/>
            <div className="create-message"></div>
          </div>
      }

      content =
        <div>
          <div className="header row">
            Create Account
          </div>
          <div className="account-information-body row">
            <div className="account-picture-wrapper">
              <AccountPicture model={accountPictureModel} onPictureChange={onAccountPictureChange}/>
            </div>
            <div className="personal-details-wrapper">
              <PersonalDetails model={personalDetailsModel} onChange={onPersonalDetailsChange}/>
            </div>
          </div>
          <div className="header row">
            User Notes
          </div>
          <div className="row">
            <UserNotes model={userNotesModel} onChange={onUserNotesChange}/>
          </div>
          {createButton}
        </div>
    }

    return (
      <div id="account-container" className="container-fixed-width">
        {content}
      </div>
    );
  }
}

export default DesktopView;
