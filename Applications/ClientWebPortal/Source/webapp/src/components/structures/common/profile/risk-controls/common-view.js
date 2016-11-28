import React from 'react';
import UpdatableView from 'commons/updatable-view';
import definitionsService from 'services/definitions';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#risk-control-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  hideSavedMessage() {
    $('#risk-control-container .save-message').stop().fadeOut();
  }

  showSavedMessage() {
    let $saveMessage = $('#risk-control-container .save-message');
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
    let $saveMessage = $('#risk-control-container .save-message');
    $saveMessage
      .fadeOut(() => {
        $saveMessage
          .text(message)
          .removeClass('purple')
          .addClass('red')
          .fadeIn();
      });
  }

  onCurrencyChange(newCode) {
    let newCurrencyNumber = definitionsService.getCurrencyNumber.apply(definitionsService, [newCode]);
    this.controller.onCurrencyChange(newCurrencyNumber);
  }
}

export default CommonView;
