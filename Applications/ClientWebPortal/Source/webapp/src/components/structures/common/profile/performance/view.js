import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import UserInfoNav from 'components/reusables/common/user-info-nav';
import ProfitAndLossPanel from 'components/reusables/common/profit-loss-panel';
import ProfitLossReportParameters from 'components/reusables/common/profit-loss-report-parameters';
import ReportStatusPanel from 'components/reusables/common/report-status-panel';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#performance-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  render() {
    let userInfoNavModel = {
      userName: this.componentModel.userName,
      roles: this.componentModel.roles
    };

    let generateBtnModel = {
      label: 'Generate'
    };

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

    let onGenerate = this.controller.generate.bind(this.controller);

    let reportHeader;
    if (this.componentModel.pnlReport != null) {
      if (deviceDetector.isMobile()) {
        reportHeader =
          <div className="report-header">
            Your P/L Report Summary<br/>{this.componentModel.startDate} - {this.componentModel.endDate}
          </div>;
      } else {
        reportHeader =
          <div className="report-header">
            Your P/L Report Summary {this.componentModel.startDate} - {this.componentModel.endDate}
          </div>;
      }
    }

    return (
      <div id="performance-container" className={className}>
        <div className="row">
          <UserInfoNav model={userInfoNavModel}/>
        </div>

        <ProfitLossReportParameters onGenerate={onGenerate}/>

        {reportHeader}
        <div className="row">
          {pnlPanels}
        </div>

        <div className="report-status-update-wrapper">
          <ReportStatusPanel/>
        </div>
      </div>
    );
  }
}

export default View;
