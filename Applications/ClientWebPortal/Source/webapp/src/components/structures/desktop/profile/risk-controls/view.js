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

  onHourValueChange(newValue) {
    newValue = newValue.toString();
    if (newValue.length == 1) {
      newValue = '0' + newValue;
    }
    let currentTime = this.componentModel.riskParameters.transitionTime;
    let updatedTime = newValue + currentTime.substring(2);
    this.componentModel.riskParameters.transitionTime = updatedTime;
    this.controller.onTransitionTimeChange.apply(this.controller, [updatedTime]);
  }

  onMinuteValueChange(newValue) {
    newValue = newValue.toString();
    if (newValue.length == 1) {
      newValue = '0' + newValue;
    }
    let currentTime = this.componentModel.riskParameters.transitionTime;
    let updatedTime = currentTime.substring(0, 3) + newValue + currentTime.substring(5);
    this.componentModel.riskParameters.transitionTime = updatedTime;
    this.controller.onTransitionTimeChange.apply(this.controller, [updatedTime]);
  }

  onSecondValueChange(newValue) {
    newValue = newValue.toString();
    if (newValue.length == 1) {
      newValue = '0' + newValue;
    }
    let currentTime = this.componentModel.riskParameters.transitionTime;
    let updatedTime = currentTime.substring(0, 6) + newValue;
    this.componentModel.riskParameters.transitionTime = updatedTime;
    this.controller.onTransitionTimeChange.apply(this.controller, [updatedTime]);
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
      if (this.componentModel.riskParameters.currencyId.value == 0) {
        saveBtnModel.isDisabled = true;
      } else {
        saveBtnModel.isDisabled = false;
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

      let buyingPowerInputModel = {
        placeholder: '0.00',
        allowedDecimals: 2,
        stepSize: 1,
        doRoll: false,
        min: 0,
        defaultValue: this.componentModel.riskParameters.buyingPower.toString(),
        isRequired: true,
        isDisabled: !this.componentModel.isAdmin
      };

      let netLossInputModel = {
        placeholder: '0.00',
        allowedDecimals: 2,
        stepSize: 1,
        doRoll: false,
        min: 0,
        defaultValue: this.componentModel.riskParameters.netLoss.toString(),
        isRequired: true,
        isDisabled: !this.componentModel.isAdmin
      };

      let timeValues = this.componentModel.riskParameters.transitionTime.split(':');

      let hourInputModel = {
        placeholder: '00',
        allowedDecimals: 0,
        stepSize: 1,
        doRoll: true,
        min: 0,
        max: 23,
        maxLength: 2,
        defaultValue: timeValues[0],
        isRequired: true,
        isDisabled: !this.componentModel.isAdmin
      };

      let minInputModel = {
        placeholder: '00',
        allowedDecimals: 0,
        stepSize: 1,
        doRoll: true,
        min: 0,
        max: 59,
        maxLength: 2,
        defaultValue: timeValues[1],
        isRequired: true,
        isDisabled: !this.componentModel.isAdmin
      };

      let secInputModel = {
        placeholder: '00',
        allowedDecimals: 0,
        stepSize: 1,
        doRoll: true,
        min: 0,
        max: 59,
        maxLength: 2,
        defaultValue: timeValues[2],
        isRequired: true,
        isDisabled: !this.componentModel.isAdmin
      };

      let currencyWrapperClass = 'currency-select-wrapper';
      if (!this.componentModel.isAdmin) {
        currencyWrapperClass += ' disabled';
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
            <div className={currencyWrapperClass}>
              <select className="currency-select"
                      defaultValue={selectedCurrency}
                      onChange={this.onCurrencyChange.bind(this)}
                      disabled={!this.componentModel.isAdmin}>
                {options}
              </select>
              <span className="icon-arrow-down"/>
            </div>
            <div className="risk-control-label">
              Buying Power <span className="currency-sign">{currencySign}</span>
            </div>
            <StepNumberInput model={buyingPowerInputModel}
                             onChange={this.onBuyingPowerChange.bind(this)}
                             onInputValidationFail={this.onBuyingPowerInputFail.bind(this)}/>
            <div className="buying-power-error validation-error"></div>
            <div className="risk-control-label">
              Net Loss <span className="currency-sign">{currencySign}</span>
            </div>
            <StepNumberInput model={netLossInputModel}
                             onChange={this.onNetLossChange.bind(this)}
                             onInputValidationFail={this.onNetLossInputFail.bind(this)}/>
            <div className="net-loss-error validation-error"></div>
            <div className="risk-control-label">
              Transition Time
            </div>
            <div className="time-input-wrapper">
              <div className="hour-input-wrapper input-wrapper">
                <StepNumberInput model={hourInputModel}
                                 onChange={this.onHourValueChange.bind(this)}/>
                <div className="time-input-label">hr</div>
              </div>
              <span className="colon">:</span>
              <div className="minute-input-wrapper input-wrapper">
                <StepNumberInput model={minInputModel}
                                 onChange={this.onMinuteValueChange.bind(this)}/>
                <div className="time-input-label">min</div>
              </div>
              <span className="colon">:</span>
              <div className="second-input-wrapper input-wrapper">
                <StepNumberInput model={secInputModel}
                                 onChange={this.onSecondValueChange.bind(this)}/>
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
