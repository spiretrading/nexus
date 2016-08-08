import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PersonalDetails from 'components/reusables/desktop/personal-details';
import AccountPicture from 'components/reusables/common/account-picture';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import moment from 'moment';
import PrimaryButton from 'components/reusables/common/primary-button';
import UserNotes from 'components/reusables/common/user-notes';
import passwordValidaotr from 'components/structures/common/profile/account/password-validator';
import userService from 'services/user';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#account-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  /** @private */
  onPasswordChangeClick() {
    let currentPassword = $('#account-container .current-password-input').val();
    let newPassword = $('#account-container .new-password-input').val();
    let confirmPassword = $('#account-container .confirm-password-input').val();

    let errorMessage = passwordValidaotr.validate(currentPassword, newPassword, confirmPassword);
    if (errorMessage != null) {
      $('#account-container .change-password-wrapper .message').text(errorMessage).css('display', 'inherit');
    } else {
      $('#account-container .change-password-wrapper .message').text("").css('display', 'none');
      this.controller.onPasswordUpdate(currentPassword, newPassword);
    }
  }

  render() {
    let content;
    let userInfoNavModel,
      lastSignin,
      personalDetailsModel,
      accountPictureModel,
      saveButtonModel,
      userNotesModel,
      changePasswordButtonModel,
      saveButton;
    if (!this.controller.isModelEmpty.apply(this.controller)) {
      userInfoNavModel = {
        userName: this.componentModel.userName,
        roles: this.componentModel.roles
      };

      personalDetailsModel = JSON.parse(JSON.stringify(this.componentModel));
      personalDetailsModel.id = personalDetailsModel.directoryEntry.id;
      delete personalDetailsModel.directoryEntry;
      personalDetailsModel.isReadOnly = !userService.isAdmin();

      lastSignin = moment(this.componentModel.lastLoginTime, moment.ISO_8601).toDate().toLocaleString();

      accountPictureModel = {
        picture: this.componentModel.picture,
        showLabel: true,
        isReadOnly: !userService.isAdmin()
      };

      userNotesModel = {
        userNotes: this.componentModel.userNotes,
        isReadOnly: !userService.isAdmin()
      };

      saveButtonModel = {
        label: 'Save Changes'
      };

      changePasswordButtonModel = {
        label: 'Change Password'
      };

      let onAccountPictureChange = this.controller.onAccountPictureChange.bind(this.controller);
      let onPersonalDetailsChange = this.controller.onPersonalDetailsChange.bind(this.controller);
      let onUserNotesChange = this.controller.onUserNotesChange.bind(this.controller);
      let onPasswordChangeClick = this.onPasswordChangeClick.bind(this);
      let save = this.controller.save.bind(this.controller);

      if (userService.isAdmin()) {
        saveButton =
          <div className="row save-button-wrapper">
            <PrimaryButton className="save-button" model={saveButtonModel} onClick={save}/>
            <div className="save-message">
              Changes saved
            </div>
          </div>
      }

      content =
        <div>
          <div className="row">
            <UserInfoNav model={userInfoNavModel}/>
          </div>
          <div className="row">
            Last sign-in: {lastSignin}
          </div>
          <div className="header row">
            Account Information
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
          {saveButton}
          <hr className="row"/>
          <div className="header row">
            Change Password
          </div>
          <div className="change-password-wrapper row">
            <div className="input-row">
              <input className="current-password-input" type="password" placeholder="Current Password"/>
              <input className="new-password-input" type="password" placeholder="New Password"/>
              <input className="confirm-password-input" type="password" placeholder="Confirm New Password"/>
              <PrimaryButton className="change-button" model={changePasswordButtonModel} onClick={onPasswordChangeClick}/>
            </div>
            <div className="message"></div>
          </div>
        </div>
    }

    return (
      <div id="account-container" className="container-fixed-width">
        {content}
      </div>
    );
  }
}

export default View;