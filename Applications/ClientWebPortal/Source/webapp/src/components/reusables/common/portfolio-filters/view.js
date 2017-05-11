import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import MoveSelectDesktop from 'components/reusables/desktop/move-select';
import MoveSelectMobile from 'components/reusables/mobile/move-select';
import PrimaryButton from 'components/reusables/common/primary-button';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selectedStartDate = null;
    this.selectedEndDate = null;
    this.selectedParameterInput = "Group";
    this.isClosed = false;
  }

  /** @private */
  onHeaderClicked() {
    let $body = $('#' + this.componentModel.componentId + ' .body');

    if (this.isClosed) {
      this.isClosed = false;
      $body.stop(true, true).animate({
        height: this.bodyHeight
      }, {
        step: function() {
          EventBus.publish(Event.Portfolio.FILTER_RESIZE);
        }
      });

      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-down').addClass('icon-arrow-up').fadeIn(200);
      });
    } else {
      this.isClosed = true;
      this.bodyHeight = $body.outerHeight();
      $body.stop(true, true).animate({
        height: '0px'
      }, {
        step: function() {
          EventBus.publish(Event.Portfolio.FILTER_RESIZE);
        }
      });

      $('#' + this.componentModel.componentId + ' .arrow-icon').stop(true, true).fadeOut(200, () => {
        $('#' + this.componentModel.componentId + ' .arrow-icon').removeClass('icon-arrow-up').addClass('icon-arrow-down').fadeIn(200);
      });
    }
  }

  /** @private */
  onParamButtonClick(e) {
    $('#' + this.componentModel.componentId + ' .body').css('height', 'auto');
    let $button = $(e.currentTarget);
    this.selectedParameterInput = $button.attr('data-name');
    this.update(this.componentModel);
  }

  /** @private */
  onMoveSelectChange(availableItems, selectedItems) {
    this.controller.onFilterChange.apply(this.controller, [this.selectedParameterInput, availableItems, selectedItems]);
  }

  render() {
    let groupClass = 'button-wrapper';
    let currencyClass = 'button-wrapper';
    let marketClass = 'button-wrapper';
    let columnClass = 'button-wrapper';
    let selectListModel = {};
    if (this.selectedParameterInput === 'Group') {
      groupClass += ' selected';
      selectListModel.subject = 'Groups';
      selectListModel.availableItems = this.componentModel.groups || [];
      selectListModel.selectedItems = this.componentModel.filter.groups || [];
    } else if (this.selectedParameterInput === 'Currency') {
      currencyClass += ' selected';
      selectListModel.subject = 'Currencies';
      selectListModel.availableItems = this.componentModel.currencies || [];
      selectListModel.selectedItems = this.componentModel.filter.currencies || [];
    } else if (this.selectedParameterInput === 'Market') {
      marketClass += ' selected';
      selectListModel.subject = 'Markets';
      selectListModel.availableItems = this.componentModel.markets || [];
      selectListModel.selectedItems = this.componentModel.filter.markets || [];
    } else if (this.selectedParameterInput === 'Column') {
      columnClass += ' selected';
      selectListModel.subject = 'Columns';
      selectListModel.availableItems = this.componentModel.columns || [];
      selectListModel.selectedItems = this.componentModel.filter.columns || [];
    }

    let updateButtonModel = {
      label: 'Update'
    };

    let update = this.controller.update.bind(this.controller);

    let groupMenu, currencyMenu, marketMenu, columnMenu;
    if (deviceDetector.isMobile()) {
      groupMenu = <span className="icon-group"/>;
      currencyMenu = <span className="icon-currency"/>;
      marketMenu = <span className="icon-market"/>;
      columnMenu = <span className="icon-column"/>;
    } else {
      groupMenu = <span><span className="icon-group"/> Group</span>;
      currencyMenu = <span><span className="icon-currency"/> Currency</span>;
      marketMenu = <span><span className="icon-market"/> Market</span>;
      columnMenu = <span><span className="icon-column"/> Column</span>;
    }

    let selectList;
    if (deviceDetector.isMobile()) {
      selectList = <MoveSelectMobile model={selectListModel} onChange={this.onMoveSelectChange.bind(this)}/>;
    } else {
      selectList = <MoveSelectDesktop model={selectListModel} onChange={this.onMoveSelectChange.bind(this)}/>;
    }

    let className = "portfolio-filters-container";
    if (this.componentModel.className != null)
    {
      className += " " + this.componentModel.className;
    }

    return (
        <div id={this.componentModel.componentId} className={className}>
          <div className="header" onClick={this.onHeaderClicked.bind(this)}>
            Filters
            <span className="icon-arrow-up arrow-icon"/>
          </div>
          <div className="body">
            <div className="content-wrapper">
              <div className="filter-buttons-wrapper">
                <div className={groupClass} data-name="Group" onClick={this.onParamButtonClick.bind(this)}>
                  {groupMenu}
                </div>
                <div className={currencyClass} data-name="Currency" onClick={this.onParamButtonClick.bind(this)}>
                  {currencyMenu}
                </div>
                <div className={marketClass} data-name="Market" onClick={this.onParamButtonClick.bind(this)}>
                  {marketMenu}
                </div>
                <div className={columnClass} data-name="Column" onClick={this.onParamButtonClick.bind(this)}>
                  {columnMenu}
                </div>
              </div>

              <div className="parameters-body">
                {selectList}
              </div>

              <PrimaryButton model={updateButtonModel} className="update-button" onClick={update}/>
            </div>
          </div>
        </div>
    );
  }
}

export default View;
