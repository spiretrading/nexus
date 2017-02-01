import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import moment from 'moment';
import PrimaryButton from 'components/reusables/common/primary-button';
import ProfitAndLossPanel from 'components/reusables/common/profit-and-loss-panel';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selectedStartDate = null;
    this.selectedEndDate = null;
  }

  componentDidUpdate() {
    $('#performance-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  initialize() {
    $('#performance-container .start-date-input').datepicker({
      changeMonth: true,
      changeYear: true,
      onSelect: (dateText, date) => {
        this.selectedStartDate = moment(dateText, 'MM/DD/YYYY');
      },
      beforeShowDay: (date) => {
        if (this.selectedEndDate == null) {
          return [true];
        }

        let showingDate = moment(date, 'ddd MMM DD YYYY');
        return [this.selectedEndDate >= showingDate];
      }
    });

    $('#performance-container .end-date-input').datepicker({
      changeMonth: true,
      changeYear: true,
      onSelect: (dateText, date) => {
        this.selectedEndDate = moment(dateText, 'MM/DD/YYYY');
      },
      beforeShowDay: (date) => {
        if (this.selectedStartDate == null) {
          return [true];
        }

        let showingDate = moment(date, 'ddd MMM DD YYYY');
        return [this.selectedStartDate <= showingDate];
      }
    });
  }

  onGenerateClick() {
    let startDate = $('#performance-container .start-date-input').val().trim();
    let endDate = $('#performance-container .end-date-input').val().trim();
    this.controller.generate(startDate, endDate);
  }

  render() {
    let userInfoNavModel = {
      userName: this.componentModel.userName,
      roles: this.componentModel.roles
    };

    let generateBtnModel = {
      label: 'Generate'
    };

    let onGenerateClick = this.onGenerateClick.bind(this);

    let pnlPanels = [];
    for (let currency in this.componentModel.pnlReport) {
      let currencyData = this.componentModel.pnlReport[currency];
      let model = {
        currency: currency,
        securities: currencyData
      };
      pnlPanels.push(<ProfitAndLossPanel key={currency} model={model}/>);
    }

    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    return (
      <div id="performance-container" className={className}>
        <div className="row">
          <UserInfoNav model={userInfoNavModel}/>
        </div>
        <div className="row parameters-wrapper">
          <div className="start-date">Start Date: <input type="text" className="start-date-input"/></div>
          <div className="end-date">End Date: <input type="text" className="end-date-input"/></div>
          <div className="generate-button-wrapper">
            <PrimaryButton className="generate-button" model={generateBtnModel} onClick={onGenerateClick}/>
          </div>
        </div>
        <div className="row">
          {pnlPanels}
        </div>
      </div>
    );
  }
}

export default View;
 