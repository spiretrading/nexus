import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#account-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  /** @private */
  validate(newPassword, confirmPassword) {
    if (newPassword == null || newPassword.length === 0) {
      return "New password is required";
    }

    if (confirmPassword == null || confirmPassword.length === 0) {
      return "Confirm password is required";
    }

    if (newPassword !== confirmPassword) {
      return "Passwords do not match";
    }

    return null;
  }

  onPasswordChangeClick() {
    let newPassword = $('#account-container .new-password-input').val();
    let confirmPassword = $('#account-container .confirm-password-input').val();
    let errorMessage = this.validate(newPassword, confirmPassword);
    if (errorMessage != null) {
      $('#account-container .change-password-wrapper .message')
        .addClass('fail')
        .text(errorMessage)
        .css('display', 'inherit');
    } else {
      $('#account-container .change-password-wrapper .message')
        .removeClass('fail')
        .text("")
        .css('display', 'none');
      this.controller.onPasswordUpdate.apply(this.controller, [newPassword]);
    }
  }

  showSavePersonalDetailsSuccessMessage() {
    $('#account-container .save-button-wrapper .save-message').text('Saved').css('display', 'inherit');
  }

  showSavePersonalDetailsFailMessage() {
    $('#account-container .save-button-wrapper .save-message').text('Failed').css('display', 'inherit');
  }

  showSavePasswordSuccess() {
    $('#account-container .change-password-wrapper .message')
      .removeClass('fail')
      .text('Saved')
      .css('display', 'inherit');

    $('#account-container .new-password-input').val('');
    $('#account-container .confirm-password-input').val('');
  }

  showSavePasswordFailMessage() {
    $('#account-container .change-password-wrapper .message')
      .addClass('fail')
      .text('Failed')
      .css('display', 'inherit');
  }
}

export default CommonView;