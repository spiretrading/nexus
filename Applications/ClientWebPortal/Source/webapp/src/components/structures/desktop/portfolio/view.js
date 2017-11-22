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
    let filter, totals, table;
    if (this.componentModel.isInitialized) {
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

      $('#portfolio-container').css('display', 'flex');

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

      filter = <PortfolioFilters model={parametersModel} onSave={onParametersSave} className={"porfolio-parameters-wrapper"}/>;
      totals =  <div className="total-wrapper">
                  <div className="total-label">Total P/L</div>
                  <div className="total-value">{totalPnL}</div>
                  <div className="total-label">Unrealized P/L</div>
                  <div className="total-value">{unrealizedPnL}</div>
                  <div className="total-label">Realized P/L</div>
                  <div className="total-value">{realizedPnL}</div>
                  <div className="total-label">Fees</div>
                  <div className="total-value">{fees}</div>
                  <div className="total-label">Volume</div>
                  <div className="total-value">{fees}</div>
                  <div className="total-label">Trades</div>
                  <div className="total-value">{trades}</div>
                  <div className="menu">
                    <div className="export-table no-select" onClick={this.controller.exportToCSV}>
                      <span className="icon-export"/>
                      Export Table
                    </div>
                    <div className="no-select">
                      <span className="icon-cancel-all-orders"/>
                      Cancel All Orders
                    </div>
                  </div>
                </div>;
      table =   <div className="table-wrapper">
                  <BigTable
                    dataModel={dataModel}
                    setReference={this.controller.setTableRef}
                    fontFamily='Roboto'
                    fontWeight='200'
                    fontSize='16px'
                    lineHeight='40'
                    changeSortOrder={this.controller.changeSortOrder}
                  />
                </div>;
    }

    return (
      <div id="portfolio-container" className="container-fixed-width">
        {filter}
        {totals}
        {table}
      </div>
    );
  }
}

export default View;
