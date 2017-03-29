import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  convertToHeaderLabel(label) {
    // convert dashes to spaces
    label = label.replace(/_/g, ' ');
    label = this.toTitleCase(label);

    if (label === 'Total Profit And Loss') {
      label = 'Total P/L';
    } else if (label === 'Unrealized Profit And Loss') {
      label = 'Unrealized P/L';
    } else if (label === 'Realized Profit And Loss') {
      label = 'Realized P/L';
    }

    return label;
  }

  toTitleCase(label) {
    return label.split(' ').map((word) => [word[0].toUpperCase(), ...word.substr(1)].join('')).join(' ');
  }

  initialize() {
    $('#' + this.componentModel.componentId).on('scroll', this.onScroll);

    this.onResize = this.onResize.bind(this);
    $(window).resize(this.onResize);
  }

  /** @private */
  onScroll(event) {
    let scrollTopOffset = $(this).scrollTop();
    let scrollLeftOffset = $(this).scrollLeft();
    $(this).find('.header').css('top', scrollTopOffset);
    $(this).find('.fixed-column').css('top', scrollTopOffset).css('left', scrollLeftOffset);
    let fixedHeaderHeight = $(this).find('.fixed-column-header').height();
    $(this).find('.fixed-column-body').css('top', fixedHeaderHeight - scrollTopOffset);

    let isScrollRight = event.originalEvent.wheelDeltaX < 0;
    let width = $(this).width();
    if ((scrollLeftOffset >= this.scrollWidth - width) && isScrollRight) {
      event.preventDefault();
    }
  }

  /** @private */
  synchronizeColumnWidths() {
    let $headerTds = $('#' + this.componentModel.componentId + ' .header td');
    let $bodyTds = $('#' + this.componentModel.componentId + '>.body tr:first td');
    let $fixColumnHeaderTds = $('#' + this.componentModel.componentId + ' .fixed-column-header td');
    let $fixColumnBodyTds = $('#' + this.componentModel.componentId + ' .fixed-column-body tr:first td');
    $bodyTds.removeAttr('style');

    for (let i=0; i<$headerTds.size(); i++) {
      let headerWidth = $($headerTds[i]).outerWidth();
      let bodyWidth = $($bodyTds[i]).outerWidth();
      let maxWidth = Math.max(headerWidth, bodyWidth);
      $($headerTds[i]).css('min-width', maxWidth);
      $($bodyTds[i]).css('min-width', maxWidth);
      $($fixColumnBodyTds[i]).css('min-width', maxWidth);
      if (i == 0) {
        $($fixColumnHeaderTds[i]).css('min-width', maxWidth);
      }
    }
  }

  /** @private */
  onResize() {
    let $container = $('#' + this.componentModel.componentId);
    let $header = $container.find('.header');
    let headerWidth = $header.outerWidth();
    let containerWidth = $container.outerWidth();
    if (headerWidth < containerWidth) {
      $header.removeClass('wide').addClass('wide');
      $container.find('.fixed-column').css('opacity', '0');
    } else {
      $header.removeClass('wide');
      $container.find('.fixed-column').css('opacity', '1');
    }
  }

  componentDidUpdate() {
    this.synchronizeColumnWidths.apply(this);
    if (this.componentModel.data != null && this.componentModel.data[0] != null) {
      $('#' + this.componentModel.componentId + ' .fixed-column-header').css('opacity', '1');
    }
  }

  dispose() {
    $('#' + this.componentModel.componentId).unbind('scroll', this.onScroll);
    $(window).unbind('resize', this.onResize);
  }

  render() {
    let columns = [];
    let columnLabels = [];
    let rows = [];
    let accountIds = [];
    let fixedBodyRows = [];

    if (this.componentModel.data != null && this.componentModel.data[0] != null) {
      for (let column in this.componentModel.data[0]) {
        columns.push(
          <td key={column}>
            {this.convertToHeaderLabel.apply(this, [column])}
          </td>
        );
        columnLabels.push(column);
      }

      for (let i=0; i<this.componentModel.data.length; i++) {
        let rowData = this.componentModel.data[i];
        accountIds.push(rowData.account);
        let columns = [];
        for (let property in rowData) {

          // get the value
          let value;
          if (property == 'account') {
            value = rowData[property].name;
          } else if (property == 'average_price' ||
            property == 'cost_basis' ||
            property == 'fees' ||
            property == 'realized_profit_and_loss' ||
            property == 'total_profit_and_loss' ||
            property == 'unrealized_profit_and_loss') {
            value = rowData[property].toNumber();
          } else if (property == 'currency') {
            value = definitionsService.getCurrencyCode(rowData[property].value);
          } else if (property == 'open_quantity' ||
            property == 'trades' ||
            property == 'volume') {
            value = numberFormatter.formatWithComma(rowData[property]);
          } else if (property == 'security') {
            value = rowData[property].symbol + '.' + rowData[property].market.value;
          }

          // set classes to color the fonts for P&L values
          let className = '';
          if (property === 'total_profit_and_loss' ||
            property === 'unrealized_profit_and_loss' ||
            property === 'realized_profit_and_loss') {
            if (value > 0) {
              className = 'profit';
            } else if (value < 0) {
              className = 'loss';
            }
          }

          columns.push(
            <td key={i + property} className={className}>
              {value}
            </td>
          );
        }

        rows.push(
          <tr key={i}>
            {columns}
          </tr>
        );
      }

      for (let i=0; i<accountIds.length; i++){
        fixedBodyRows.push(
          <tr key={i}>
            <td className="body-cell">{accountIds[i].name}</td>
          </tr>
        );
      }

    }

    return (
        <div id={this.componentModel.componentId} className="portfolio-chart-container">

          <div className="header">
            <table>
              <thead>
                <tr>
                  {columns}
                </tr>
              </thead>
            </table>
          </div>

          <div className="fixed-column">
            <div className="fixed-column-header">
              <table>
                <tbody>
                  <tr>
                    <td>Account</td>
                  </tr>
                </tbody>
              </table>
            </div>
            <div className="fixed-column-body">
              <table>
                <tbody>
                  {fixedBodyRows}
                </tbody>
              </table>
            </div>
          </div>

          <div className="header-filler"></div>

          <div className="body">
            <table>
              <tbody>
                {rows}
              </tbody>
            </table>
          </div>
        </div>
    );
  }
}

export default View;
