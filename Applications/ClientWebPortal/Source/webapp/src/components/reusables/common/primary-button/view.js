import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let className = 'primary-button-container';
    if (this.componentModel.className != null) {
      className += ' ' + this.componentModel.className;
    }

    let onButtonClick = this.controller.onButtonClick.bind(this.controller);

    return (
        <div id={this.componentModel.componentId} className={className} onClick={onButtonClick}>
          {this.componentModel.label}
        </div>
    );
  }
}

export default View;
