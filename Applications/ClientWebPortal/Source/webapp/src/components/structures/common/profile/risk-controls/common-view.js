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

  showSavedMessage() {
    $('#risk-control-container .save-message')
      .text('Saved')
      .removeClass('failed')
      .css('display', 'inherit');
  }

  showSaveFailedMessage() {
    $('#risk-control-container .save-message')
      .text('Failed')
      .removeClass('failed')
      .addClass('failed')
      .css('display', 'inherit');
  }

  onCurrencyChange(newCode) {
    let newCurrencyNumber = definitionsService.getCurrencyNumber.apply(definitionsService, [newCode]);
    this.controller.onCurrencyChange(newCurrencyNumber);
  }
}

export default CommonView;