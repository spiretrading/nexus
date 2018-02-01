import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import ChangePicture from 'components/reusables/common/change-picture';
import modal from 'utils/modal';
import Dropdown from 'components/reusables/common/dropdown';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.onCloseChangePicClick = this.onCloseChangePicClick.bind(this);

    this.changePictureModel = {
      picture: this.componentModel.picture,
      shouldDispose: false
    };
  }

  /** @private */
  onChangeButtonClick() {
    modal.show($('#change-picture-modal'));
  }

  /** @private */
  onChangePictureSubmit(isChanged, pictureData, onSubmitted) {
    modal.hide($('#change-picture-modal'), onSubmitted);

    if (isChanged) {
      this.controller.onAccountPictureChange(pictureData);
    }
  }

  /** @private */
  onCloseChangePicClick() {
    modal.hide($('#change-picture-modal'));
    this.changePictureModel.shouldDispose = true;
    this.update();
  }

  dispose() {
    modal.hide($('#change-picture-modal'));
  }

  render() {
    let label;
    if (this.componentModel.showLabel) {
      label =
        <div className="labels-container">
          Account Picture
        </div>
    }

    let buttonStyle;
    if (this.componentModel.isReadOnly) {
      buttonStyle = {
        display: "none"
      };
    } else {
      buttonStyle = {
        display: "initial"
      };
    }

    let changeButton;
    if (!this.componentModel.isReadOnly) {
      changeButton =
        <div className="picture-change-btn"  onClick={this.onChangeButtonClick}>
          <span className="icon-change-image" style={buttonStyle}></span>
        </div>
    }

    let picture;
    if (this.componentModel.picture == null || this.componentModel.picture === '') {
      picture =
        <div className="default-img">
          <span className="icon-photo"/>
        </div>
    } else {
      picture = <img src={this.componentModel.picture}/>
    }

    let onChangePictureSubmit = this.onChangePictureSubmit.bind(this);

    return (
        <div className="account-picture-container">
          {label}
          <div className="picture-container">
            {picture}
            {changeButton}
          </div>

          <div id="change-picture-modal" className="modal fade" tabIndex="-1" role="dialog">
            <div className="modal-dialog" role="document">
              <div className="modal-content">
                <div className="modal-header">
                  Change Picture
                  <span className="icon-close" onClick={this.onCloseChangePicClick}></span>
                </div>
                <div className="modal-body change-picture-wrapper">
                  <ChangePicture model={this.changePictureModel} onSubmit={onChangePictureSubmit}/>
                </div>
              </div>
            </div>
          </div>
        </div>
    );
  }
}

export default View;
