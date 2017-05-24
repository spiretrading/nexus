import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PortfolioFilters from 'components/reusables/common/portfolio-filters';
import PortfolioChart from 'components/reusables/common/portfolio-chart';
import numberFormatter from 'utils/number-formatter';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  initialize() {
    $(window).resize(this.onWindowResize.bind(this));
  }

  /** @private */
  onWindowResize() {
    this.resizePortfolioChart.apply(this);
  }

  resizePortfolioChart() {
    let chartOuterWrapperOffset = $('#portfolio-container .chart-outer-wrapper').offset();
    $('#portfolio-container .chart-inner-wrapper').css('left', -1 * chartOuterWrapperOffset.left);

    let $container = $('#portfolio-container');
    let $chartOuterWrapper = $('#portfolio-container .chart-outer-wrapper');
    if ($chartOuterWrapper.outerWidth() < $container.outerWidth()) {
      $('#portfolio-container .chart-inner-wrapper').css('width', $container.outerWidth());
    } else {
      $('#portfolio-container .chart-inner-wrapper').css('width', '100%');
    }

    // $chartOuterWrapper.removeClass('stretch').addClass('stretch');
    let $portfolioParametersWrapper = $container.find('.porfolio-parameters-wrapper');
    let chartOuterWrapperHeight = $container.height() - $portfolioParametersWrapper.height();
    $chartOuterWrapper.height(chartOuterWrapperHeight);
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
    };

    let onParametersSave = this.controller.saveParameters.bind(this.controller);

    if (this.controller.isModelInitialized()) {
      $('#portfolio-container').css('display', 'flex');
    }

    let chartModel = {
      data: this.componentModel.portfolioData,
      filter: this.componentModel.filter
    };

    let totalPnL, unrealizedPnL, realizedPnL, fees, volumes, trades;
    if (this.componentModel.aggregates != null) {
      totalPnL = numberFormatter.formatTwoDecimalsWithComma.apply(numberFormatter, [this.componentModel.aggregates.totalPnL.toNumber()]);
      unrealizedPnL = numberFormatter.formatTwoDecimalsWithComma.apply(numberFormatter, [this.componentModel.aggregates.unrealizedPnL.toNumber()]);
      realizedPnL = numberFormatter.formatTwoDecimalsWithComma.apply(numberFormatter, [this.componentModel.aggregates.realizedPnL.toNumber()]);
      fees = numberFormatter.formatTwoDecimalsWithComma.apply(numberFormatter, [this.componentModel.aggregates.realizedPnL.toNumber()]);
      volumes = numberFormatter.formatWithComma.apply(numberFormatter, [this.componentModel.aggregates.volumes]);
      trades = numberFormatter.formatWithComma.apply(numberFormatter, [this.componentModel.aggregates.trades]);
    }

    return (
      <div id="portfolio-container" className="container-fixed-width">
        <PortfolioFilters model={parametersModel} onSave={onParametersSave} className={"porfolio-parameters-wrapper"}/>
        <div className="total-wrapper">
          <div>Total P/L {totalPnL}</div>
          <div>Unrealized P/L {unrealizedPnL}</div>
          <div>Realized P/L {realizedPnL}</div>
          <div>Fees {fees}</div>
          <div>Volume {volumes}</div>
          <div>Trades {trades}</div>
        </div>
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
