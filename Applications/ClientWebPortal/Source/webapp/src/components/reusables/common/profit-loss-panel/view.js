import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import numberFormatter from 'utils/number-formatter';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onExpandClick(event) {
    let $contentWrapper = $(event.currentTarget).parent().parent().find('.content-wrapper');
    let $contentSlideWrapper = $(event.currentTarget).parent().parent().find('.content-slide-wrapper');
    let contentHeight = $contentWrapper.height() + 20;
    let contentSlideWrapperHeight = $contentSlideWrapper.height();
    if (contentSlideWrapperHeight == 0) {
      $contentSlideWrapper.stop().animate({
        height: contentHeight
      });
      $(event.currentTarget).removeClass('collapsed').addClass('expanded');
    } else {
      $contentSlideWrapper.stop().animate({
        height: 0
      });
      $(event.currentTarget).removeClass('expanded').addClass('collapsed');
    }
  }

  render() {
    let tableHeader, tableBody;
    tableHeader =
      <thead>
        <tr>
          <td>Security</td>
          <td>P/L</td>
          <td>Volume</td>
        </tr>
      </thead>;

    let securities = [];
    let totalPnl = 0, totalVolume = 0;
    for (let i=0; i<this.componentModel.securities.length; i++) {
      totalPnl += this.componentModel.securities[i].pnl;
      totalVolume += this.componentModel.securities[i].volume;

      let trClassName = "";
      if (i == 0) {
        trClassName += ' first-row';
      }
      if (i == this.componentModel.securities.length - 1) {
        trClassName += ' last-row';
      }

      let pnl = currencyFormatter.formatByCode.apply(currencyFormatter, [
        this.componentModel.currency,
        this.componentModel.securities[i].pnl
      ]);

      securities.push(
        <tr key={i} className={trClassName}>
          <td>{this.componentModel.securities[i].symbol}</td>
          <td>{pnl}</td>
          <td>{numberFormatter.formatWithComma(this.componentModel.securities[i].volume)}</td>
        </tr>
      );
    }
    tableBody =
      <tbody>
        {securities}
      </tbody>;

    let componentContainerClassName = 'profit-loss-panel-container';

    let totalPnLValueClassName = 'value-field';
    if (totalPnl > 0) {
      totalPnLValueClassName += ' profit';
    } else {
      totalPnLValueClassName += ' loss';
    }

    totalPnl = currencyFormatter.formatByCode.apply(currencyFormatter, [
      this.componentModel.currency,
      totalPnl
    ]);

    totalVolume = numberFormatter.formatWithComma(totalVolume);

    let header;
    if (!deviceDetector.isMobile()) {
      header =
        <div className="header">
          <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
          <span className="currency">
                <div>Currency</div>
                <div className="value-field">{this.componentModel.currency}</div>
          </span>
          <span className="pnl-total">
                <div>Total P/L</div>
                <div className={totalPnLValueClassName}>
                  {totalPnl}
                </div>
          </span>
          <span className="volume-total">
                <div>Volume</div>
                <div className="value-field">{totalVolume}</div>
          </span>
        </div>
    } else {
      header =
        <div className="header">
          <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
          <table>
            <tbody>
              <tr>
                <td className="label-cell">Currency</td>
                <td className="value-field">{this.componentModel.currency}</td>
              </tr>
              <tr>
                <td className="label-cell">Total P/L</td>
                <td className={totalPnLValueClassName}>{totalPnl}</td>
              </tr>
              <tr>
                <td className="label-cell">Volumes</td>
                <td className="value-field">{totalVolume}</td>
              </tr>
            </tbody>
          </table>
        </div>
    }

    return (
        <div id={this.componentModel.componentId} className={componentContainerClassName}>
          {header}

          <div className="content-slide-wrapper">
            <div className="content-wrapper">
              <table>
                {tableHeader}
                {tableBody}
              </table>
            </div>
          </div>
        </div>
    );
  }
}

export default View;
