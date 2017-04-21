import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  onButtonClick() {
    if (!this.componentModel.isDisabled) {
      this.controller.onButtonClick.apply(this.controller);
    }
  }

  render() {
    let className = 'primary-button-container';
    if (this.componentModel.className != null) {
      className += ' ' + this.componentModel.className;
    }

    if (!this.componentModel.isDisabled) {
      className += ' enabled';
    }

    let onButtonClick = this.onButtonClick.bind(this);

    return (
        <div id={this.componentModel.componentId} className={className} onClick={onButtonClick}>
          {this.componentModel.label}
        </div>
    );
  }
}

export default View;
