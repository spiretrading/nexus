import './style.scss';
import React from 'react';
import deviceDetector from 'utils/device-detector';
import PrimaryButton from 'components/reusables/common/primary-button';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import definitionsService from 'services/definitions';
import StepNumberInput from 'components/reusables/common/step-number-input';
import CommonView from 'components/structures/common/profile/risk-controls/common-view';

class View extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onMoneyInputKeyUp(event) {
    let $input = $(event.currentTarget);
    let input = $input.val();
    let $error = $('#risk-control-container').find('.' + $input.attr('data-control') + '-error');
    let validationFailMessage = this.validateInput.apply(this, [input, 2]);

    if (validationFailMessage == null) {
      $input.removeClass('invalid').addClass('valid');
      $error.text('');
      if ($input.attr('data-control') == 'buying-power') {
        this.controller.onBuyingPowerChange(Number(input));
      } else if ($input.attr('data-control') == 'net-loss') {
        this.controller.onNetLossChange(Number(input));
      }
    } else {
      $input.removeClass('valid').addClass('invalid');
      $error.text(validationFailMessage);
    }
  }

  /** @private */
  onTimeInputKeyUp(event) {
    let $input = $(event.currentTarget);
    let input = $input.val();
    let isHour = $input.hasClass('hour');
    let validationFailMessage = this.validateInput.apply(this, [input, 0, true, isHour]);

    if (validationFailMessage == null) {
      $input.removeClass('invalid').addClass('valid');

      // update the model with new valid value
      if (input.length == 1) {
        input = '0' + input;
      }
      let currentTime = this.componentModel.riskParameters.transitionTime;
      let updatedTime;
      if ($input.hasClass('hour')) {
        updatedTime = newValue + currentTime.substring(2);
      } else if ($input.hasClass('minute')) {
        updatedTime = currentTime.substring(0, 3) + newValue + currentTime.substring(5);
      } else {
        updatedTime = currentTime.substring(0, 6) + newValue;
      }
      this.componentModel.riskParameters.transitionTime = updatedTime;
      this.controller.onTransitionTimeChange.apply(this.controller, [updatedTime]);
    } else {
      $input.removeClass('valid').addClass('invalid');
    }
  }

  /** @private */
  validateInput(input, numDecimalPlaces, isTime, isHour) {
    let inputNumber = Number(input);
    if (isNaN(inputNumber)) {
      return 'Not a number.';
    } else if (!this.isValidDecimals.apply(this, [input, numDecimalPlaces]) && numDecimalPlaces == 0) {
      return 'Decimal places are not allowed.';
    } else if (!this.isValidDecimals.apply(this, [input, numDecimalPlaces])) {
      return 'Exceeded ' + numDecimalPlaces + ' decimal places.';
    } else if (isTime && !isHour && inputNumber > 59) {
      return 'Cannot be more than 59 minutes.';
    } else if (input == null || input.length == 0) {
      return 'This field is required.';
    }

    return null;
  }

  /** @private */
  isValidDecimals(input, numDecimalPlaces) {
    let periodIndex = input.indexOf('.');
    if (periodIndex == -1) {
      return true;
    } else if (periodIndex > -1 && numDecimalPlaces == 0) {
      return false;
    } else {
      let decimalPlaces = input.substring(periodIndex + 1);
      return decimalPlaces.length <= numDecimalPlaces;
    }
  }

  render() {
    let content;
    let containerClassName;
    if (!deviceDetector.isMobile()) {
      containerClassName = 'container-fixed-width';
    } else {
      containerClassName = '';
    }

    let userInfoNavModel;
    if (this.controller.isModelInitialized.apply(this.controller)) {
      let currencySign = definitionsService.getCurrencySign(this.componentModel.riskParameters.currencyId.value);
      if (currencySign != '') {
        currencySign = '(' + currencySign + ')';
      }

      userInfoNavModel = {
        userName: this.componentModel.directoryEntry.name,
        roles: this.componentModel.roles
      };

      let onSave = this.onSaveClick.bind(this);
      let saveBtnModel = {
        label: 'Save Changes'
      };
      if (this.componentModel.riskParameters.currencyId.value != 0) {
        saveBtnModel.isEnabled = true;
      }

      let saveButton;
      if (this.componentModel.isAdmin) {
        saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>
      }

      let currencies = definitionsService.getAllCurrencies();
      let options = [];
      options.push(
        <option key={-1} value="-1" disabled>Select a currency</option>
      );
      for (let i=0; i<currencies.length; i++) {
        let id = currencies[i].id.value;
        let code = currencies[i].code;
        options.push(
          <option key={i} value={id}>
            {code}
          </option>
        );
      }
      let selectedCurrency;
      if (this.componentModel.riskParameters.currencyId.value == 0) {
        selectedCurrency = -1;
      } else {
        selectedCurrency = this.componentModel.riskParameters.currencyId.value;
      }

      content =
        <div>
          <div className="user-info-nav-wrapper">
            <UserInfoNav model={userInfoNavModel}/>
          </div>
          <div className="form-wrapper">
            <div className="risk-control-label">
              Currency
            </div>
            <div className="currency-select-wrapper">
              <select className="currency-select"
                      defaultValue={selectedCurrency}
                      onChange={this.onCurrencyChange.bind(this)}>
                {options}
              </select>
              <span className="icon-arrow-down"/>
            </div>
            <div className="risk-control-label">
              Buying Power <span className="currency-sign">{currencySign}</span>
            </div>
            <input type="text" className="buying-power-input money-input"
                   placeholder="0.00"
                   data-control="buying-power"
                   onKeyUp={this.onMoneyInputKeyUp.bind(this)}/>
            <div className="buying-power-error validation-error"></div>
            <div className="risk-control-label">
              Net Loss <span className="currency-sign">{currencySign}</span>
            </div>
            <input type="text" className="net-loss-input money-input"
                   placeholder="0.00"
                   data-control="net-loss"
                   onKeyUp={this.onMoneyInputKeyUp.bind(this)}/>
            <div className="net-loss-error validation-error"></div>
            <div className="risk-control-label">
              Transition Time
            </div>
            <div className="time-input-wrapper">
              <div className="hour-input-wrapper input-wrapper">
                <input type="text" className="hour time-input"
                       placeholder="00"
                       maxLength="2"
                       onKeyUp={this.onTimeInputKeyUp.bind(this)}/>
                <div className="time-input-label">hr</div>
              </div>
              <span className="colon">:</span>
              <div className="minute-input-wrapper input-wrapper">
                <input type="text" className="minute time-input"
                       placeholder="00"
                       maxLength="2"
                       onKeyUp={this.onTimeInputKeyUp.bind(this)}/>
                <div className="time-input-label">min</div>
              </div>
              <span className="colon">:</span>
              <div className="second-input-wrapper input-wrapper">
                <input type="text" className="second time-input"
                       placeholder="00"
                       maxLength="2"
                       onKeyUp={this.onTimeInputKeyUp.bind(this)}/>
                <div className="time-input-label">sec</div>
              </div>
            </div>
            {saveButton}
            <div className="save-message"></div>
          </div>
        </div>
    }

    return (
      <div id="risk-control-container" className={containerClassName}>
        {content}
      </div>
    );
  }
}

export default View;
