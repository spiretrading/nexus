import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PortfolioFilters from 'components/reusables/common/portfolio-filters';
import PortfolioChart from 'components/reusables/common/portfolio-chart';
import tableColumns from 'components/structures/common/portfolio/table-columns';
import numberFormatter from 'utils/number-formatter';
import BigTable from 'components/reusables/common/big-table';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onWindowResize = this.onWindowResize.bind(this);
    this.initialize = this.initialize.bind(this);
    this.dispose = this.dispose.bind(this);
    this.onExportCSVClick = this.onExportCSVClick.bind(this);
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
  
  onExportCSVClick() {
    this.controller.exportToCSV();
  }

  render() {
    let filterColumns = this.getNonPrimaryKeyColumns();
    let parametersModel = {
      groups: this.componentModel.managedGroups || [],
      currencies: this.componentModel.currencies || [],
      markets: this.componentModel.markets || [],
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

    let chartModel = {
      data: this.componentModel.portfolioData,
      filter: this.componentModel.filter
    };

    let totalPnL, unrealizedPnL, realizedPnL, fees, volumes, trades;
    if (this.componentModel.aggregates != null) {
      totalPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.totalPnL.toNumber());
      unrealizedPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.unrealizedPnL.toNumber());
      realizedPnL = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.realizedPnL.toNumber());
      fees = numberFormatter.formatTwoDecimalsWithComma(this.componentModel.aggregates.realizedPnL.toNumber());
      volumes = numberFormatter.formatWithComma(this.componentModel.aggregates.volumes);
      trades = numberFormatter.formatWithComma(this.componentModel.aggregates.trades);
    }

    let dataModel = this.controller.getDataModel();

    return (
      <div id="portfolio-container" className="container">
        <PortfolioFilters model={parametersModel} onSave={onParametersSave}/>
        <div className="total-wrapper">
          <table>
            <tbody>
              <tr>
                <td>Total P/L<br/>{totalPnL}</td>
                <td>Unrealized P/L<br/>{unrealizedPnL}</td>
              </tr>
              <tr>
                <td>Realized P/L<br/>{realizedPnL}</td>
                <td>Fees<br/>{fees}</td>
              </tr>
              <tr>
                <td>Volume<br/>{volumes}</td>
                <td>Trades<br/>{trades}</td>
              </tr>
            </tbody>
          </table>
        </div>
        <div className="table-wrapper">
          <BigTable
            dataModel={dataModel}
            columnTypes={tableColumns}
            setReference={this.controller.setTableRef}
            fontFamily='Roboto'
            fontWeight='200'
            fontSize='16px'
            lineHeight='40'
            changeSortOrder={this.controller.changeSortOrder}
          />
        </div>
        <div className="export-btn" onClick={this.onExportCSVClick}>Export to CSV</div>
      </div>
    );
  }
}

export default View;
