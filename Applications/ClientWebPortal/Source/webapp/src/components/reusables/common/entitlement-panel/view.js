import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
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

  /** @priavte */
  onCheckClick(event) {
    if (this.componentModel.isAdmin) {
      let $panelContainer = $(event.currentTarget).parent().parent();
      if ($panelContainer.hasClass('selected')) {
        this.controller.onEntitlementDeselected();
      } else {
        this.controller.onEntitlementSelected();
      }
    }
  }

  render() {
    let priceLabel;
    if (this.componentModel.entitlement.price == 0) {
      priceLabel = 'FREE';
    } else {
      let price = this.componentModel.entitlement.price / 1000000;
      let formattedAmount = currencyFormatter.formatById(this.componentModel.entitlement.currency, price);
      let currencyCode = definitionsService.getCurrencyCode(this.componentModel.entitlement.currency);
      priceLabel = formattedAmount + " " + currencyCode;
    }

    let tableHeader, tableBody;
    if (deviceDetector.isMobile()) {
      tableHeader =
        <thead>
        <tr>
          <td>Mkt</td>
          <td>BBO</td>
          <td>MQ</td>
          <td>BQ</td>
          <td>T&S</td>
          <td>Imb</td>
        </tr>
        </thead>
    } else {
      tableHeader =
        <thead>
          <tr>
            <td>Market</td>
            <td>BBO</td>
            <td>Market Quotes</td>
            <td>Book Quotes</td>
            <td>Time & Sale</td>
            <td>Imbalances</td>
          </tr>
        </thead>
    }

    let markets = [];
    for (let i=0; i<this.componentModel.entitlement.applicability.length; i++) {
      let applicability = this.componentModel.entitlement.applicability[i];
      let provisions = applicability[1];
      let imbalance = ((provisions & (1 << 4)) != 0);
      let timeSale = ((provisions & (1 << 0)) != 0);
      let bookQuotes = ((provisions & (1 << 1)) != 0);
      let marketQuotes = ((provisions & (1 << 2)) != 0);
      let bbo = ((provisions & (1 << 3)) != 0);

      let greyDotClassName = 'icon-dot';
      let greenDotClassName = 'icon-dot green';
      let imbalanceClassName = (imbalance ? greenDotClassName : greyDotClassName);
      let timeSaleClassName = (timeSale ? greenDotClassName : greyDotClassName);
      let bookQuotesClassName = (bookQuotes ? greenDotClassName : greyDotClassName);
      let marketQuotesClassName = (marketQuotes ? greenDotClassName : greyDotClassName);
      let bboClassName = (bbo ? greenDotClassName : greyDotClassName);

      let trClassName = "";
      if (i == 0) {
        trClassName += ' first-row';
      }
      if (i == this.componentModel.entitlement.applicability.length - 1) {
        trClassName += ' last-row';
      }

      markets.push(
        <tr key={i} className={trClassName}>
          <td className="market-name">{applicability[0].source}</td>
          <td><span className={bboClassName}></span></td>
          <td><span className={marketQuotesClassName}></span></td>
          <td><span className={bookQuotesClassName}></span></td>
          <td><span className={timeSaleClassName}></span></td>
          <td><span className={imbalanceClassName}></span></td>
        </tr>
      );
    }
    tableBody =
      <tbody>
        {markets}
      </tbody>

    let componentContainerClassName = 'entitlement-panel-container';
    if (this.componentModel.isSelected) {
      componentContainerClassName += ' selected';
    }

    return (
        <div id={this.componentModel.componentId} className={componentContainerClassName}>
          <div className="header">
            <span className="icon-check" onClick={this.onCheckClick.bind(this)}></span>
            <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
            <span className="title">{this.componentModel.entitlement.name}</span>
            <span className="price">{priceLabel}</span>
          </div>
          <div className="content-slide-wrapper">
            <div className="content-wrapper">
              <div className="content-header">
                Applicability
                <div className="price">
                  {priceLabel}
                </div>
              </div>
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
