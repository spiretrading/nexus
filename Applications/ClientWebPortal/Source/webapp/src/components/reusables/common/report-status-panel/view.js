import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import ReportType from './report-type';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selectedStartDate = null;
    this.selectedEndDate = null;
    this.isClosed = false;
    this.screenHeightWithoutHeader;

    this.initialize = this.initialize.bind(this);
    this.dispose = this.dispose.bind(this);
  }

  initialize() {
    $(window).resize(this.onWindowResize.bind(this));
    this.screenHeightWithoutHeader = window.innerHeight - 32;
  }

  dispose() {
    $(window).off('resize', this.onWindowResize.bind(this));
  }

  /** @private */
  onWindowResize() {
    this.screenHeightWithoutHeader = window.innerHeight - 32;
    this.initializeCotainerHeight();
  }

  /** @private */
  onBarClick() {
    let $container = $('#' + this.componentModel.componentId);
    if ($container.hasClass('closed')) {
      let height;
      if (deviceDetector.isMobile()) {
        height = this.screenHeightWithoutHeader + 'px';
      } else {
        height = '450px';
      }

      $container.find('.body').stop(true, true).animate({
        height: height
      });
      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-up').addClass('icon-arrow-down').fadeIn(200);
      });
      $container.removeClass('closed');
    } else {
      $container.find('.body').stop(true, true).animate({
        height: '0px'
      });
      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-down').addClass('icon-arrow-up').fadeIn(200);
      });
      $container.addClass('closed');
    }
  }

  /** @private */
  initializeCotainerHeight() {
    let height;
    if (deviceDetector.isMobile()) {
      height = this.screenHeightWithoutHeader + 'px';
    } else {
      height = '450px';
    }

    if (deviceDetector.isMobile()) {
      $('#' + this.componentModel.componentId + ' .body ul').css({
        'height': height,
        'overflow-y': 'scroll',
        '-webkit-overflow-scrolling': 'touch'
      });
    } else {
      $('#' + this.componentModel.componentId + ' .body ul').slimScroll({
        height: height,
        opacity: 0
      }).mouseover(function() {
        $(this).next('.slimScrollBar').css('opacity', 0.4);
      });
    }
  }

  componentDidUpdate() {
    setTimeout(() => {
      $('#' + this.componentModel.componentId + ' .description-wrapper').ellipsis({
        lines: 2,
        ellipClass: 'ellip'
      });
    }, 0);

    this.initializeCotainerHeight();
  }

  render() {
    let statusItems = [];

    if (this.componentModel.reportStatuses != null) {
      for (let i=0; i<this.componentModel.reportStatuses.length; i++) {
        let iconClass, typeLabel;
        let description = '';
        if (this.componentModel.reportStatuses[i].type === ReportType.PROFIT_LOSS) {
          iconClass = 'icon-reports';
          typeLabel = 'P/L Report';
        } else if (this.componentModel.reportStatuses[i].type === ReportType.OTHER) {
          iconClass = 'icon-reports';
          typeLabel = 'Other Report';
        }

        let parameters = this.componentModel.reportStatuses[i].parameters;
        for (let name in parameters) {
          description += name + ': ' + parameters[name] + ', ';
        }
        description = description.substring(0, description.length - 2);

        if (!deviceDetector.isMobile()) {
          statusItems.push(
            <li key={i}>
              <div className="item-container">
                <div className="icon-wrapper">
                  <span className={iconClass}/>
                </div>
                <div className="type-label-wrapper">
                  {typeLabel}
                </div>
                <div className="description-wrapper">
                  {description}
                </div>
                <div className="status-wrapper">
                  {this.componentModel.reportStatuses[i].status}
                </div>
              </div>
            </li>
          );
        } else {
          statusItems.push(
            <li key={i}>
              <div className="item-container">
                <div className="icon-wrapper">
                  <span className={iconClass}/>
                </div>
                <div className="content-wrapper">
                  <div className="upper-wrapper">
                    <div className="type-label-wrapper">
                      {typeLabel}
                    </div>
                    <div className="status-wrapper">
                      {this.componentModel.reportStatuses[i].status}
                    </div>
                  </div>
                  <div className="description-wrapper">
                    {description}
                  </div>
                </div>
              </div>
            </li>
          );
        }

      }
    }

    return (
        <div id={this.componentModel.componentId} className="report-status-panel-container closed" onClick={this.onBarClick.bind(this)}>
          <div className="header no-select">
            Check Report Status
            <span className="arrow-icon icon-arrow-up"/>
          </div>
          <div className="body">
            <ul>
              {statusItems}
            </ul>
          </div>
        </div>
    );
  }
}

export default View;
