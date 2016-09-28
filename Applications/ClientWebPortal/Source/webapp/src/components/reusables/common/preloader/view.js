import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.minDurationTimeout = null;
    this.isNeededToHide = false;
  }

  /** @private */
  onHidden() {
    this.controller.notifyPreloaderHidden();
  }

  show() {
    this.minDurationTimeout = setTimeout(() => {
      this.minDurationTimeout = null;
      if (this.isNeededToHide) {
        this.hide();
      }
    }, Config.MIN_PRELOADER_DURATION);

    $('.' + this.componentModel.id + ' .animation-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  hide() {
    if (this.minDurationTimeout == null) {
      let onHidden = this.onHidden.bind(this);
      $('.' + this.componentModel.id + ' .animation-container').fadeOut({
        duration: Config.FADE_DURATION,
        complete: onHidden
      });
    } else {
      this.isNeededToHide = true;
    }
  }

  render() {
    let className = this.componentModel.id + ' preloader-container';
    let preloader;
    let style = {
      width: this.componentModel.width,
      height: this.componentModel.height
    };
    if (deviceDetector.isInternetExplorer()){
      preloader = <img src="/images/preloader.gif" style={style}/>
    } else {
      preloader = <object data="/images/preloader.svg" type="image/svg+xml" style={style}></object>
    }

    return (
        <div className={className}>
          <div className="animation-container">
            {preloader}
          </div>
        </div>
    );
  }
}

export default View;
