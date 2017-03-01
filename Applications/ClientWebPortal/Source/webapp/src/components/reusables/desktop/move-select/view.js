import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onAddClick() {
    let addingItems = [];
    $('#' + this.componentModel.componentId + ' .available-options option:selected').each(function() {
      addingItems.push($(this).attr('value'));
    });

    $('#' + this.componentModel.componentId + ' option:selected').removeAttr('selected');

    if (addingItems.length > 0) {
      this.controller.addItems.apply(this.controller, [addingItems]);
    }
  }

  /** @private */
  onAddAllClick() {
    $('#' + this.componentModel.componentId + ' option:selected').removeAttr('selected');
    this.controller.addAllItems.apply(this.controller);
  }

  /** @private */
  onRemoveClick() {
    let removingItems = [];
    $('#' + this.componentModel.componentId + ' .selected-options option:selected').each(function() {
      removingItems.push($(this).attr('value'));
    });

    $('#' + this.componentModel.componentId + ' option:selected').removeAttr('selected');

    if (removingItems.length > 0) {
      this.controller.removeItems.apply(this.controller, [removingItems]);
    }
  }

  /** @private */
  onRemoveAllClick() {
    $('#' + this.componentModel.componentId + ' option:selected').removeAttr('selected');
    this.controller.removeAllItems.apply(this.controller);
  }

  render() {
    let availableOptions = [];
    let selectedOptions = [];

    for (let i=0; i<this.componentModel.availableItems.length; i++) {
      let name = this.componentModel.availableItems[i].name;
      let id = this.componentModel.availableItems[i].id;
      availableOptions.push(
        <option key={i} value={id}>{name}</option>
      );
    }

    for (let i=0; i<this.componentModel.selectedItems.length; i++) {
      let name = this.componentModel.selectedItems[i].name;
      let id = this.componentModel.selectedItems[i].id;
      selectedOptions.push(
        <option key={i} value={id}>{name}</option>
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
              <select multiple className="available-options options">
                {availableOptions}
              </select>
            </div>
          </div>

          <div className="desktop-buttons">
            <div className="add-button move-select-button" onClick={this.onAddClick.bind(this)}>
              <span className="icon-single-arrow"></span>
            </div>
            <div className="add-all-button move-select-button" onClick={this.onAddAllClick.bind(this)}>
              <span className="icon-double-arrow"></span>
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
              <select multiple className="selected-options options">
                {selectedOptions}
              </select>
            </div>
          </div>

        </div>
    );
  }
}

export default View;
