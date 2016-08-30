import React from 'react';
import './style.scss';
import CommonView from 'components/structures/common/profile/risk-controls/common-view';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import DropDownCard from 'components/reusables/mobile/cards/drop-down-card';
import CurrencyCard from 'components/reusables/common/cards/currency-card';
import TimeCard from 'components/reusables/common/cards/time-card';
import PrimaryButton from 'components/reusables/common/primary-button';
import deviceDetector from 'utils/device-detector';
import definitionsService from 'services/definitions';

class DesktopView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let content;
    let containerClassName;
    if (!deviceDetector.isMobile()) {
      containerClassName = 'container-fixed-width';
    } else {
      containerClassName = '';
    }

    if (!this.controller.isModelEmpty.apply(this.controller)) {
      let userInfoNavModel = {
        userName: this.componentModel.userName,
        roles: this.componentModel.roles
      };

      let currencyModel, netLossModel, buyingPowerModel, transitionTimeModel;
      if (definitionsService.doesCurrencyExist.apply(definitionsService, [this.componentModel.riskParameters.currency])) {
        currencyModel = {
          title: 'Currency',
          value: definitionsService.getCurrencyCode.apply(definitionsService, [this.componentModel.riskParameters.currency]),
          options: definitionsService.getAllCurrencyCodes.apply(definitionsService),
          isReadOnly: !this.componentModel.isAdmin
        };

        netLossModel = {
          title: 'Net Loss',
          value: this.componentModel.riskParameters.netLoss,
          countryIso: this.componentModel.riskParameters.currency,
          isReadOnly: !this.componentModel.isAdmin
        };

        buyingPowerModel = {
          title: 'Buying Power',
          value: this.componentModel.riskParameters.buyingPower,
          countryIso: this.componentModel.riskParameters.currency,
          isReadOnly: !this.componentModel.isAdmin
        };

        transitionTimeModel = {
          title: 'Transition Time',
          value: this.componentModel.riskParameters.transitionTime,
          isReadOnly: !this.componentModel.isAdmin
        };
      } else {
        currencyModel = {
          title: 'Currency',
          value: 'None',
          options: definitionsService.getAllCurrencyCodes.apply(definitionsService),
          isReadOnly: !this.componentModel.isAdmin
        };

        netLossModel = {
          title: 'Net Loss',
          value: this.componentModel.riskParameters.netLoss,
          countryIso: this.componentModel.riskParameters.currency,
          isReadOnly: !this.componentModel.isAdmin
        };

        buyingPowerModel = {
          title: 'Buying Power',
          value: this.componentModel.riskParameters.buyingPower,
          countryIso: this.componentModel.riskParameters.currency,
          isReadOnly: !this.componentModel.isAdmin
        };

        transitionTimeModel = {
          title: 'Transition Time',
          value: this.componentModel.riskParameters.transitionTime,
          isReadOnly: !this.componentModel.isAdmin
        };
      }

      let onCurrencyChange = this.onCurrencyChange.bind(this);
      let onNetLossChange = this.controller.onNetLossChange.bind(this.controller);
      let onBuyingPowerChange = this.controller.onBuyingPowerChange.bind(this.controller);
      let onTransitionTimeChange = this.controller.onTransitionTimeChange.bind(this.controller);
      let onSave = this.controller.save.bind(this.controller);

      let saveBtnModel = {
        label: 'Save Changes'
      };
      let saveButton, horizontalDivider;
      if (this.componentModel.isAdmin) {
        saveButton = <PrimaryButton className="save-button" model={saveBtnModel} onClick={onSave}/>
        horizontalDivider = <hr/>
      }

      content =
        <div>
          <div className="row">
            <UserInfoNav model={userInfoNavModel}/>
          </div>
          <div className="row cards-wrapper">
            <div className="card-wrapper">
              <DropDownCard model={currencyModel} onChange={onCurrencyChange}/>
            </div>
            <div className="card-wrapper">
              <CurrencyCard model={netLossModel} onChange={onNetLossChange}/>
            </div>
            <div className="card-wrapper">
              <CurrencyCard model={buyingPowerModel} onChange={onBuyingPowerChange}/>
            </div>
            <div className="card-wrapper last-card">
              <TimeCard model={transitionTimeModel} onChange={onTransitionTimeChange}/>
            </div>
          </div>
          {horizontalDivider}
          {saveButton}
          <div className="save-message"></div>
        </div>
    }

    return (
      <div id="risk-control-container" className={containerClassName}>
        {content}
      </div>
    );
  }
}

export default DesktopView;
