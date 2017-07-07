import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.deselectAll = this.deselectAll.bind(this);
  }

  /** @private */
  onAddClick() {
    let addingItems = [];
    $('#' + this.componentModel.componentId + ' .available-list-wrapper .selected-item').each(function() {
      addingItems.push($(this).attr('value'));
    });

    this.deselectAll();

    if (addingItems.length > 0) {
      this.controller.addItems(addingItems);
    }
  }

  /** @private */
  onAddAllClick() {
    this.deselectAll();
    this.controller.addAllItems();
  }

  /** @private */
  onRemoveClick() {
    let removingItems = [];
    $('#' + this.componentModel.componentId + ' .selected-list-wrapper .selected-item').each(function() {
      removingItems.push($(this).attr('value'));
    });

    this.deselectAll();

    if (removingItems.length > 0) {
      this.controller.removeItems(removingItems);
    }
  }

  /** @private */
  onRemoveAllClick() {
    this.deselectAll();
    this.controller.removeAllItems();
  }

  /** @private */
  onItemTouch(e) {
    let $touched = $(e.currentTarget);
    if ($touched.hasClass('selected-item')) {
      $touched.removeClass('selected-item');
    } else {
      $touched.addClass('selected-item');
    }
  }

  deselectAll() {
    $('#' + this.componentModel.componentId + ' .selected-item').removeClass('selected-item');
  }

  render() {
    let availableOptions = [];
    let selectedOptions = [];

    for (let i=0; i<this.componentModel.availableItems.length; i++) {
      let name = this.componentModel.availableItems[i].name;
      let id = this.componentModel.availableItems[i].id;
      availableOptions.push(
        <li key={i} value={id} className="no-select" onClick={this.onItemTouch.bind(this)}>{name}</li>
      );
    }

    for (let i=0; i<this.componentModel.selectedItems.length; i++) {
      let name = this.componentModel.selectedItems[i].name;
      let id = this.componentModel.selectedItems[i].id;
      selectedOptions.push(
        <li key={i} value={id} className="no-select" onClick={this.onItemTouch.bind(this)}>{name}</li>
      );
    }

    let addButtonModel = {
      label: 'Add'
    };

    let addAllButtonModel = {
      label: 'Add All'
    };

    let removeButtonModel = {
      label: 'Remove'
    };

    let removeAllButtonModel = {
      label: 'Remove All'
    };

    return (
        <div id={this.componentModel.componentId} className="move-select-container">

          <div className="available-list-wrapper list-wrapper">
            <div className="list-label">Available {this.componentModel.subject}</div>
            <div className="select-wrapper">
              <ul>
                {availableOptions}
              </ul>
            </div>
          </div>

          <div className="buttons-wrapper">
            <div className="add-all-button move-select-button" onClick={this.onAddAllClick.bind(this)}>
              <span className="icon-double-arrow"></span>
            </div>
            <div className="add-button move-select-button" onClick={this.onAddClick.bind(this)}>
              <span className="icon-single-arrow"></span>
            </div>
            <div className="remove-button move-select-button" onClick={this.onRemoveClick.bind(this)}>
              <span className="icon-single-arrow"></span>
            </div>
            <div className="remove-all-button move-select-button" onClick={this.onRemoveAllClick.bind(this)}>
              <span className="icon-double-arrow"></span>
            </div>
          </div>

          <div className="selected-list-wrapper list-wrapper">
            <div className="list-label">Selected {this.componentModel.subject}</div>
            <div className="select-wrapper">
              <ul>
                {selectedOptions}
              </ul>
            </div>
          </div>

        </div>
    );
  }
}

export default View;
