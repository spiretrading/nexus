import './style.scss';
import React from 'react';
import ReactDOM from 'react-dom';
import UpdatableView from 'commons/updatable-view';
import PrimaryButton from 'components/reusables/common/primary-button';
import Croppie from 'croppie';
import 'components/../../node_modules/croppie/croppie.css';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.cropper = {
      instance: null
    };
  }

  /** @private */
  onCancelClick() {
    this.controller.cancel(onCancelled.bind(this));

    function onCancelled() {
      $('#' + this.componentModel.componentId + ' .file-input').val('');
      this.destroyCroopper();
      this.renderStaticPicture();
    }
  }

  /** @private */
  onSaveClick() {
    if (this.cropper.instance != null) {
      this.cropper.instance.result('canvas').then((imageData) => {
        this.destroyCroopper();
        this.controller.save(imageData);
      });
    } else {
      this.controller.save('');
    }
  }

  /** @private */
  destroyCroopper() {
    if (this.cropper.instance != null) {
      this.cropper.instance.destroy();
      this.cropper.instance = null;
    }
  }

  /** @private */
  onBrowseClick() {
    $('#' + this.componentModel.componentId + ' .file-input').click();
  }

  /** @private */
  initializeCrop() {
    var componentId = this.componentId;
    var cropper = this.cropper;
    if (this.files && this.files[0]) {
      if (cropper.instance != null) {
        cropper.instance.destroy();
      }

      var fileReader= new FileReader();
      fileReader.onload = function(event) {
        let $croppieContainer = $('#' + componentId + ' .edit-picture-container');
        ReactDOM.unmountComponentAtNode($croppieContainer[0]);
        cropper.instance = new Croppie($croppieContainer[0], {
          viewport: {
            width: 300,
            height: 189,
            type: 'square'
          },
          boundary: {
            width: 300,
            height: 189
          },
          exif: true
        });
        cropper.instance.bind({
          url: event.target.result
        });
      };
      fileReader.readAsDataURL( this.files[0] );
    }
  }

  /** @private */
  renderStaticPicture() {
    let container = $('#' + this.componentModel.componentId + ' .edit-picture-container')[0];
    let picture;
    if (this.componentModel.picture == null || this.componentModel.picture === '') {
      picture =
        <div className="default-img">
          <span className="icon-photo"/>
        </div>
    } else {
      picture = <img className="static-picture" src={this.componentModel.picture}/>
    }
    ReactDOM.render(picture, container);
  }

  initialize() {
    var componentId = this.componentModel.componentId;
    var cropper = this.cropper;
    var initializeCrop = this.initializeCrop;
    $('#' + componentId + ' .file-input')[0].addEventListener('change', onFileSelect, false);

    function onFileSelect() {
      if ($('#' + componentId + ' .file-input').val() != '') {
        let context = this;
        context.componentId = componentId;
        context.cropper = cropper;
        initializeCrop.apply(context);
      }
    }

    this.renderStaticPicture();
  }

  componentDidUpdate() {
    this.renderStaticPicture();
  }

  render() {
    let choosePictureBtnModel = {
      label: 'Browse'
    };

    let saveBtnModel = {
      label: 'Save'
    };

    let cancelBtnModel = {
      label: 'Cancel'
    };

    let onCancelClick = this.onCancelClick.bind(this);
    let onSaveClick = this.onSaveClick.bind(this);
    let onBrowseClick = this.onBrowseClick.bind(this);

    return (
        <div id={this.componentModel.componentId} className="change-picture-container">
          <div className="edit-picture-container"></div>
          <div className="buttons-container">
            <PrimaryButton className="button" model={choosePictureBtnModel} onClick={onBrowseClick}/>
            <PrimaryButton className="button" model={saveBtnModel} onClick={onSaveClick}/>
            <PrimaryButton className="button" model={cancelBtnModel} onClick={onCancelClick}/>
          </div>
          <input type="file" className="file-input"/>
        </div>
    );
  }
}

export default View;