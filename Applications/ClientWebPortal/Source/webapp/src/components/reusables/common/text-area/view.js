import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.inputTimeout = null;
  }

  /** @private */
  onChange(event) {
    let inputText = event.currentTarget.value.trim();
    if (this.inputTimeout != null) {
      clearTimeout(this.inputTimeout);
    }

    this.inputTimeout = setTimeout(() => {
      this.inputTimeout = null;
      this.controller.onInputChange(inputText);
    }, Config.INPUT_TIMEOUT_DURATION);
  }

  render() {
    let textarea;
    if (this.componentModel.isReadOnly) {
      textarea = <textarea className="text-area-input"
                           defaultValue={this.componentModel.text}
                           placeholder={this.componentModel.placeHolder}
                           readOnly />
    } else {
      textarea = <textarea className="text-area-input"
                           defaultValue={this.componentModel.text}
                           placeholder={this.componentModel.placeHolder}
                           onChange={this.onChange.bind(this)}/>
    }

    return (
        <div id={this.componentModel.componentId} className="text-area-container">
          {textarea}
        </div>
    );
  }
}

export default View;
