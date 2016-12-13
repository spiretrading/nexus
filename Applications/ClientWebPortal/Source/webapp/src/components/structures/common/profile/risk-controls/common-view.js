import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import definitionsService from 'services/definitions';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  onBuyingPowerInputFail(failMessage) {
    $('#risk-control-container .buying-power-error').text(failMessage);
  }

  onBuyingPowerChange(newValue) {
    $('#risk-control-container .buying-power-error').text('');
    this.controller.onBuyingPowerChange(newValue);
  }

  onNetLossInputFail(failMessage) {
    $('#risk-control-container .net-loss-error').text(failMessage);
  }

  onNetLossChange(newValue) {
    $('#risk-control-container .net-loss-error').text('');
    this.controller.onNetLossChange(newValue);
  }

  onCurrencyChange() {
    let currencyId = $('#risk-control-container select.currency-select').val();
    this.controller.onCurrencyChange.apply(this.controller, [currencyId]);
    let currencySign = definitionsService.getCurrencySign(currencyId);
    $('#risk-control-container .currency-sign').text('(' + currencySign + ')');
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

  onSaveClick() {
    if ($('#risk-control-container .invalid').length == 0) {
      this.controller.save.apply(this.controller);
    } else {
      this.showSaveFailedMessage('Failed due to invalid input(s)');
    }
  }
}

export default CommonView;
