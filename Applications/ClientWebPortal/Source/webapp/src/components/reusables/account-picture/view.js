import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let label;
    if (this.componentModel.showLabel) {
      label =
        <div className="labels-container">
          Account Picture
        </div>
    }

    return (
        <div className="account-picture-container">
          {label}
          <div className="picture-container">
            <img src={this.componentModel.pictureUrl}/>
          </div>
          <div className="picture-change-btn">
            <span className="icon-change_image"></span>
          </div>
        </div>
    );
  }
}

export default View;