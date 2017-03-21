import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';

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
    $('#' + this.componentModel.componentId).scroll(this.onScroll);

    this.onResize = this.onResize.bind(this);
    $(window).resize(this.onResize);
  }

  /** @private */
  onScroll() {
    let scrollTopOffset = $(this).scrollTop();
    $(this).find('.header').css('top', scrollTopOffset);
  }

  /** @private */
  synchronizeColumnWidths() {
    let $headerTds = $('#' + this.componentModel.componentId + ' .header td');
    let $bodyTds = $('#' + this.componentModel.componentId + ' .body tr:first td');
    $bodyTds.removeAttr('style');

    for (let i=0; i<$headerTds.size(); i++) {
      let headerWidth = $($headerTds[i]).outerWidth();
      let bodyWidth = $($bodyTds[i]).outerWidth();
      let maxWidth = Math.max(headerWidth, bodyWidth);
      $($headerTds[i]).css('min-width', maxWidth);
      $($bodyTds[i]).css('min-width', maxWidth);
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
    } else {
      $header.removeClass('wide');
    }
  }

  componentDidUpdate() {
    this.synchronizeColumnWidths.apply(this);
  }

  dispose() {
    $('#' + this.componentModel.componentId).unbind('scroll', this.onScroll);
    $(window).unbind('resize', this.onResize);
  }

  render() {
    let columns = [];
    let columnLabels = [];
    let rows = [];
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
        let columns = [];
        for (let property in rowData) {
          let value = rowData[property];

          let className = '';
          if (property === 'total_profit_and_loss' ||
            property === 'unrealized_profit_and_loss' ||
            property === 'realized_profit_and_loss') {
            if (value > 0) {
              className = 'profit';
            } else {
              className = 'loss';
            }
          }

          if (property === 'average_price' ||
            property === 'total_profit_and_loss' ||
            property === 'unrealized_profit_and_loss' ||
            property === 'realized_profit_and_loss' ||
            property === 'fees' ||
            property === 'cost_basis') {
            value = currencyFormatter.formatByCode(rowData.currency, value);
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
    }

    return (
        <div id={this.componentModel.componentId} className="sticky-header-chart-container">
          <div className="header">
            <table>
              <thead>
                <tr>
                  {columns}
                </tr>
              </thead>
            </table>
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
