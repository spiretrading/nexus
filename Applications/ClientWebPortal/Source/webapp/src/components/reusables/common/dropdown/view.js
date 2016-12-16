import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onSelectionChange(event) {
    let newValue = $('#' + this.componentModel.componentId + ' select').val();
    this.controller.onSelectionChange.apply(this.controller, [newValue]);
  }

  render() {
    let className = this.componentModel.className + ' dropdown-container';

    if (this.componentModel.isDisabled) {
      className += ' disabled';
    }

    let options = [];
    for (let i=0; i<this.componentModel.options.length; i++) {
      let optionModel = this.componentModel.options[i];
      options.push(
        <option key={i} value={optionModel.value}>{optionModel.label}</option>
      );
    }

    let onSelectionChange = this.onSelectionChange.bind(this);

    return (
        <div id={this.componentModel.componentId} className={className} onChange={onSelectionChange}>
          <select className={this.componentModel.className}
                  defaultValue={this.componentModel.selectedValue}
                  disabled={this.componentModel.isDisabled}>
            {options}
          </select>
          <span className="icon-arrow-down"/>
        </div>
    );
  }
}

export default View;
