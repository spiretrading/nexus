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
      let $saveMessage = $('#account-container .change-password-wrapper .message');
      $saveMessage
        .fadeOut(() => {
          $saveMessage
            .text(errorMessage)
            .removeClass('purple')
            .addClass('red')
            .fadeIn();
        });
    } else {
      $('#account-container .change-password-wrapper .message')
        .fadeOut(() => {
          this.controller.onPasswordUpdate.apply(this.controller, [newPassword]);
        });
    }
  }

  hideSavePersonalDetailsMessage() {
    $('#account-container .save-button-wrapper .save-message').stop().fadeOut();
  }

  showSavePersonalDetailsSuccessMessage() {
    let $saveMessage = $('#account-container .save-button-wrapper .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Saved')
          .removeClass('red')
          .addClass('purple')
          .fadeIn();
      });
  }

  showSavePersonalDetailsFailMessage() {
    let $saveMessage = $('#account-container .save-button-wrapper .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Failed')
          .removeClass('purple')
          .addClass('red')
          .fadeIn();
      });
  }

  hideSavePasswordMessage() {
    $('#account-container .change-password-wrapper .message').stop().fadeOut();
  }

  showSavePasswordSuccess() {
    let $saveMessage = $('#account-container .change-password-wrapper .message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Saved')
          .removeClass('red')
          .addClass('purple')
          .fadeIn();
      });
  }

  resetInputs() {
    $('#account-container .new-password-input').val('');
    $('#account-container .confirm-password-input').val('');
  }

  showSavePasswordFailMessage() {
    let $saveMessage = $('#account-container .change-password-wrapper .message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text('Failed')
          .removeClass('purple')
          .addClass('red')
          .fadeIn();
      });
  }

  hideAccountProfile() {
    $('#account-container').css('display', 'none');
  }
}

export default CommonView;
