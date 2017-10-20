import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PortfolioFilters from 'components/reusables/common/portfolio-filters';
import tableColumns from 'components/structures/common/portfolio/table-columns';
import numberFormatter from 'utils/number-formatter';
import BigTable from 'components/reusables/common/big-table';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onWindowResize = this.onWindowResize.bind(this);
    this.initialize = this.initialize.bind(this);
    this.dispose = this.dispose.bind(this);
  }

  initialize() {
    $(window).resize(this.onWindowResize);
  }

  /** @private */
  onWindowResize() {
    this.controller.resizeTable();
  }

  dispose() {
    $(window).off('resize', this.onWindowResize);
  }

  componentDidUpdate() {
    $('#portfolio-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  getNonPrimaryKeyColumns() {
    let columns = [];
    let clonedColumns = clone(tableColumns);
    for (let i=0; i<clonedColumns.length; i++) {
      if (!clonedColumns[i].isPrimaryKey) {
        columns.push(clonedColumns[i]);
      }
    }
    return columns;
  }

  render() {
    let filterColumns = this.getNonPrimaryKeyColumns();
    let parametersModel = {
      groups: [],
      currencies: [],
      markets: [],
      filter: {
        columns: filterColumns,
        currencies: this.componentModel.currencies || [],
        groups: this.componentModel.managedGroups || [],
        markets: this.componentModel.markets || []
      }
    };

    let onParametersSave = this.controller.saveParameters.bind(this.controller);

    if (this.controller.isModelInitialized()) {
      $('#portfolio-container').css('display', 'flex');
    }

    let totalPnL, unrealizedPnL, realizedPnL, fees, volumes, trades;
    if (this.componentModel.aggregates != null && this.componentModel.aggregates.totalPnL != null) {
      totalPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.totalPnL.toNumber());
      unrealizedPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.unrealizedPnL.toNumber());
      realizedPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.realizedPnL.toNumber());
      fees = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.fees.toNumber());
      volumes = numberFormatter.formatWithComma(this.componentModel.aggregates.volumes);
      trades = numberFormatter.formatWithComma(this.componentModel.aggregates.trades);
    }

    let dataModel = this.controller.getDataModel();

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
        <div className="table-wrapper">
          <BigTable
            dataModel={dataModel}
            setReference={this.controller.setTableRef}
            fontFamily='Roboto'
            fontWeight='200'
            fontSize='16px'
            lineHeight='40'
            changeSortOrder={this.controller.changeSortOrder}
          />
        </div>
      </div>
    );
  }
}

export default View;
