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
    let userNotes = event.currentTarget.value.trim();
    if (this.inputTimeout != null) {
      clearTimeout(this.inputTimeout);
    }

    this.inputTimeout = setTimeout(() => {
      this.inputTimeout = null;
      this.controller.onUserNotesChange(userNotes);
    }, Config.INPUT_TIMEOUT_DURATION);
  }

  render() {
    let textarea;
    if (this.componentModel.isReadOnly) {
      textarea = <textarea className="user-notes-input"
                           defaultValue={this.componentModel.userNotes}
                           readOnly />
    } else {
      textarea = <textarea className="user-notes-input"
                           defaultValue={this.componentModel.userNotes}
                           onChange={this.onChange.bind(this)}/>
    }

    return (
        <div id={this.componentModel.componentId} className="user-notes-container">
          {textarea}
        </div>
    );
  }
}

export default View;
