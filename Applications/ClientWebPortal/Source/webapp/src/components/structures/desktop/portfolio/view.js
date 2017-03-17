import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PortfolioParameters from 'components/reusables/common/portfolio-filters';
import PortfolioChart from 'components/reusables/common/portfolio-chart';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  initialize() {
    $(window).resize(this.onWindowResize);
  }

  /** @private */
  onWindowResize() {
    let chartOuterWrapperOffset = $('#portfolio-container .chart-outer-wrapper').offset();
    $('#portfolio-container .chart-inner-wrapper').css('left', -1 * chartOuterWrapperOffset.left);
  }

  dispose() {
    $(window).unbind('resize', this.onWindowResize);
  }

  componentDidUpdate() {
    $('#portfolio-container').fadeIn({
      duration: Config.FADE_DURATION
    });
    $(window).resize();
  }

  render() {
    let parametersModel = {
      groups: this.componentModel.managedGroups || [],
      currencies: this.componentModel.currencies || [],
      markets: this.componentModel.markets || [],
      filter: {}
    }

    let onParametersSave = this.controller.saveParameters.bind(this.controller);

    if (this.controller.isModelInitialized()) {
      $('#portfolio-container').css('display', 'flex');
    }

    let chartModel = this.componentModel.portfolioData;

    return (
      <div id="portfolio-container" className="container-fixed-width">
        <PortfolioParameters model={parametersModel} onSave={onParametersSave}/>
        <div className="chart-outer-wrapper">
          <div className="chart-inner-wrapper">
            <PortfolioChart model={chartModel} />
          </div>
        </div>
      </div>
    );
  }
}

export default View;
