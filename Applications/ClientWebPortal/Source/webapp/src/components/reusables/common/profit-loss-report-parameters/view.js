import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PrimaryButton from 'components/reusables/common/primary-button';
import moment from 'moment';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selectedStartDate = null;
    this.selectedEndDate = null;
    this.isClosed = false;

    this.initialize = this.initialize.bind(this);
  }

  initialize() {
    $('#' + this.componentModel.componentId + ' .start-date-input').datepicker({
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

    $('#' + this.componentModel.componentId + ' .end-date-input').datepicker({
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

  /** @private */
  onArrowClicked() {
    let height;
    if (deviceDetector.isMobile()) {
      height = '240px';
    } else {
      height = '96px';
    }

    if (this.isClosed) {
      this.isClosed = false;
      $('#' + this.componentModel.componentId + ' .body').stop(true, true).animate({
        height: height
      });

      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-down').addClass('icon-arrow-up').fadeIn(200);
      });
    } else {
      this.isClosed = true;
      $('#' + this.componentModel.componentId + ' .body').stop(true, true).animate({
        height: '0px'
      });

      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-up').addClass('icon-arrow-down').fadeIn(200);
      });
    }
  }

  /** @private */
  onGenerateClick() {
    let startDate = $('#' + this.componentModel.componentId + ' .start-date-input').val().trim();
    let endDate = $('#' + this.componentModel.componentId + ' .end-date-input').val().trim();
    this.controller.generate(startDate, endDate);
  }

  render() {
    let generateButtonModel = {
      label: 'Generate Report'
    };

    let headerLabel;
    if (deviceDetector.isMobile()) {
      headerLabel = 'P/L Report Parameters';
    } else {
      headerLabel = 'Profit and Loss Report Parameters';
    }

    return (
        <div id={this.componentModel.componentId} className="profit-loss-report-parameters-container">
          <div className="header">
            {headerLabel}
            <span className="icon-arrow-up arrow-icon" onClick={this.onArrowClicked.bind(this)} />
          </div>
          <div className="body">
            <div className="content-wrapper">
              <div className="start-date-wrapper">
                <div className="input-label">Start Date</div>
                <input type="text" className="start-date-input date-input"/>
              </div>
              <div className="end-date-wrapper">
                <div className="input-label">End Date</div>
                <input type="text" className="end-date-input date-input"/>
              </div>
              <div className="button-wrapper">
                <PrimaryButton className="generate-button" model={generateButtonModel} onClick={this.onGenerateClick.bind(this)}/>
              </div>
            </div>
          </div>
        </div>
    );
  }
}

export default View;
