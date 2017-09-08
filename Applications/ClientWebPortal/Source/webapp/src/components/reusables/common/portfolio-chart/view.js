import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import chartColumns from './columns';
import {CurrencyId, Money} from 'spire-client';
import HashMap from 'hashmap';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.columnsSyncCounter = 0;
    this.selectedRows = new HashMap();
    this.isCntrlPressed = false;
    this.isShiftPressed = false;
    this.activeRowId = null;
  }

  initialize() {
    // $('#' + this.componentModel.componentId).on('scroll', this.onScroll);
    //
    // this.onResize = this.onResize.bind(this);
    // $(window).resize(this.onResize);
    // $(document).keydown(this.onKeyDownCheckSelectionModifiers)
    //   .keyup(this.onKeyUpCheckSelectionModifiers);
  }

  dispose() {
    // $('#' + this.componentModel.componentId).unbind('scroll', this.onScroll);
    // $(window).unbind('resize', this.onResize);
    // $(document).unbind('keydown', this.onKeyDownCheckSelectionModifiers)
    //   .unbind('keyup', this.onKeyUpCheckSelectionModifiers);
  }

  /** @private */
  onRowClick(e) {
    let $row = $(e.currentTarget);
    let clickedRowId = $row.attr('data-row-id');
    let selectTargetRowId;

    if (!this.isCntrlPressed && !this.isShiftPressed) {
      // when no modifiers are pressed
      this.activeRowId = clickedRowId;
      this.selectedRows.clear();
      this.selectedRows.set(clickedRowId, false);
    } else if (this.isCntrlPressed && !this.isShiftPressed) {
      // when only the control modifier is pressed
      this.activeRowId = clickedRowId;
      if (this.selectedRows.has(clickedRowId)) {
        this.selectedRows.remove(clickedRowId);
      } else {
        this.selectedRows.set(clickedRowId, true);

        // set all existing selected rows with control modifier applied
        this.selectedRows.forEach((value, key) => {
          this.selectedRows.set(key, true);
        });
      }
    } else {
      // when shift modifier is pressed
      let $allRows = $row.parent().children();
      let pivotRowsMetCounter = 0;
      let model = this;

      // unselect the selected rows not chosen by control modifier
      let selectedRowsClone = model.selectedRows.clone();
      selectedRowsClone.forEach(function(value, key) {
        if (!value) {
          model.selectedRows.remove(key);
        }
      });

      $allRows.each(function() {
        if (model.activeRowId == null) {
          model.activeRowId = clickedRowId;
        }

        let $currentRow = $(this);
        let currentRowId = $currentRow.attr('data-row-id');
        if (currentRowId === model.activeRowId || currentRowId === clickedRowId) {
          pivotRowsMetCounter++;
        }

        if (pivotRowsMetCounter > 0 && pivotRowsMetCounter <= 2) {
          model.selectedRows.set(currentRowId, false);

          if (pivotRowsMetCounter == 2) {
            pivotRowsMetCounter++;
          }
        }
      });
    }

    this.react.forceUpdate();
  }

  /** @private */
  convertToHeaderLabel(label) {
    if (label === 'totalPnL') {
      label = 'Total P/L';
    } else if (label === 'unrealizedPnL') {
      label = 'Unrealized P/L';
    } else if (label === 'realizedPnL') {
      label = 'Realized P/L';
    } else if (label !== 'Total P/L' &&
               label !== 'Unrealized P/L' &&
               label !== 'Realized P/L' &&
               label !== 'Acc. Total P/L' &&
               label !== 'Acc. Unrealized P/L' &&
               label !== 'Acc. Fees') {
      label = this.convertToTitle(label);
    }

    return label;
  }

  /** @private */
  convertToTitle(label) {
    label = label.replace(/([A-Z]+)*([A-Z][a-z])/g, "$1$2");
    label = label.charAt(0).toUpperCase() + label.slice(1);
    label = label.replace(/([A-Z])/g, ' $1').trim().replace('  ', ' ');
    return label;
  }

  /** @private */
  shouldIncludeColumn(columnHeader) {
    if (this.componentModel.filter == null ||
      columnHeader == 'Account' ||
      columnHeader == 'Security' ||
      columnHeader.indexOf('Account ') >= 0
    ) {
      return true;
    }

    for (let i=0; i<this.componentModel.filter.length; i++) {
      if (columnHeader == this.componentModel.filter[i].name) {
        return true;
      }
    }

    return false;
  }

  /** @private */
  onHeaderClick(event) {
    let clickedColumn = event.currentTarget.innerText.trim();
    if (this.componentModel.sortingColumn == null || this.componentModel.sortingColumn.name != clickedColumn) {
      this.componentModel.sortingColumn = {
        name: clickedColumn,
        direction: 'asc'
      };
    } else if (this.componentModel.sortingColumn.direction == 'asc') {
      this.componentModel.sortingColumn.direction = 'desc';
    } else if (this.componentModel.sortingColumn.direction == 'desc') {
      this.componentModel.sortingColumn = null;
    }

    this.react.forceUpdate();
  }

  /** @private */
  getSecurityLabel(security) {
    let marketDatabase = definitionsService.getMarketDatabase();
    return security.toString(marketDatabase);
  }

  render() {
    let headerColumns=[];
    let rows=[];

    // add column headers
    for (let i=0; i<chartColumns.length; i++) {
      let column = chartColumns[i];
      let columnHeader = this.convertToHeaderLabel(column.name);
      if (this.shouldIncludeColumn(columnHeader)) {
        let arrowIcon;
        if (this.componentModel.sortingColumn != null && this.componentModel.sortingColumn.name == columnHeader) {
          let arrowIconClass;
          if (this.componentModel.sortingColumn.direction == 'asc') {
            arrowIconClass = 'icon-arrow-icon-down';
          } else if (this.componentModel.sortingColumn.direction == 'desc') {
            arrowIconClass = 'icon-arrow-icon-up';
          }
          arrowIcon = <span className={arrowIconClass}></span>;
        }
        headerColumns.push(
          <td key={column.id} className="no-select" onClick={this.onHeaderClick.bind(this)}>
            {columnHeader}
            {arrowIcon}
          </td>
        );
      }
    }

    // rows of the table body
    if (this.componentModel.data != null && this.componentModel.data[0] != null) {
      for (let i=0; i<this.componentModel.data.length; i++) {
        let rowData = this.componentModel.data[i];
        let columns = [];

        for (let property in rowData) {
          // get the value
          let value;
          let rawAmount;
          if (property == 'account') {
            value = rowData[property].name;
          } else if (property == 'averagePrice' ||
            property == 'costBasis' ||
            property == 'fees' ||
            property == 'realizedPnL' ||
            property == 'totalPnL' ||
            property == 'unrealizedPnL' ||
            property == 'accountTotalPnL' ||
            property == 'accountUnrealizedPnL' ||
            property == 'accountFees') {
            if (rowData[property] == null) {
              value = 'N/A';
            } else {
              if (property == 'costBasis') {
                rawAmount = rowData[property].toString();
                if (rawAmount < 0) {
                  rawAmount *= -1;
                }
              } else {
                rawAmount = rowData[property].toString();
              }
              value = currencyFormatter.formatById(rowData.currency.toNumber(), rawAmount);
            }
          } else if (property == 'currency') {
            value = definitionsService.getCurrencyCode(rowData[property].value);
          } else if (property == 'quantity' ||
            property == 'trades' ||
            property == 'volume') {
            value = numberFormatter.formatWithComma(Math.abs(rowData[property]));
          } else if (property == 'security') {
            value = this.getSecurityLabel(rowData[property]);
          } else {
            value = rowData[property];
          }

          // set classes to color the fonts for P&L values
          let className = '';
          if (property === 'totalPnL' ||
            property === 'unrealizedPnL' ||
            property === 'realizedPnL') {
            if (rawAmount > 0) {
              className = 'profit';
            } else if (rawAmount < 0) {
              className = 'loss';
            }
          }

          let columnHeader = this.convertToHeaderLabel(property);
          if (this.shouldIncludeColumn(columnHeader)) {
            columns.push(
              <td key={i + property} className={className}>
                {value}
              </td>
            );
          }
        }

        let accountId = rowData.account.id;
        let securitySymbol = rowData.security.symbol;
        let securityMarket = rowData.security.market.value;
        let id = accountId + '-' + securitySymbol + '-' + securityMarket;

        let rowClass = 'no-select';
        if (this.selectedRows.has(id)) {
          rowClass += ' selected';
        }

        rows.push(
          <tr key={i} className={rowClass}
            onClick={this.onRowClick.bind(this)}
            data-row-id={id}>
            {columns}
          </tr>
        );
      }
    }


    return (
        <div id={this.componentModel.componentId} className="portfolio-chart-container">

          <table>
            <thead>
              <tr>
                {headerColumns}
              </tr>
            </thead>
            <tbody>
              {rows}
            </tbody>
          </table>

        </div>
    );
  }
}

export default View;
