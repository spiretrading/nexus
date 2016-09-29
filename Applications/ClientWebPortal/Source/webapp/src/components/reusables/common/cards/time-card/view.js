import '../style.scss';
import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import timeFormatter from 'utils/time-formatter';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selected = false;
  }

  initialize() {
    if (!this.componentModel.isReadOnly) {
      $(document).on('click.' + this.componentModel.componentId, this.onDocumentClick.bind(this));
    }

    this.isEnabled = this.componentModel.isEnabled;
    let $componentContainer = $('#' + this.componentModel.componentId);
    if (this.isEnabled) {
      $componentContainer.click(this.onCardClick.bind(this));
    } else {
      $componentContainer.find('input').prop('disabled', true);
    }

    $componentContainer.find('input').click((event) => {
      if (!this.selected) {
        return true;
      } else {
        return false;
      }
    }).on('keydown', (event) => {
      if (isNumber() || isAllowedKeys()) {
        return true;
      } else {
        return false;
      }

      function isNumber() {
        if ((event.keyCode >= 48 && event.keyCode <= 57) || (event.keyCode >= 96 && event.keyCode <= 105)) {
          return true;
        } else {
          return false;
        }
      }

      function isAllowedKeys() {
        if (
          event.keyCode === 8           // backspace
          || event.keyCode === 46       // delete
          || event.keyCode === 37       // arrow left
          || event.keyCode === 39       // arrow right
          || event.keyCode === 35       // end
          || event.keyCode === 36       // home
          || event.keyCode === 9        // tab
        ) {
          return true;
        } else {
          return false;
        }
      }
    });
  }

  enable() {
    if (!this.isEnabled) {
      this.componentModel.isEnabled = true;
      let $componentContainer = $('#' + this.componentModel.componentId);
      $componentContainer.click(this.onCardClick.bind(this));
      $componentContainer.find('input').prop('disabled', false);
    }
  }

  dispose() {
    if (!this.componentModel.isReadOnly) {
      $(document).off('click.' + this.componentModel.componentId);
    }
  }

  /** @private */
  onDocumentClick(event) {
    let cardContainerElement = $('#' + this.componentModel.componentId)[0];
    let clickedElement = event.target;
    let isCardClicked = $.contains(cardContainerElement, clickedElement) || cardContainerElement === clickedElement;

    if (!isCardClicked && this.selected) {
      this.selected = false;
      this.onDeselected();
    }
  }

  /** @private */
  onSelected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.addClass('selected');
  }

  /** @private */
  onDeselected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
    $cardContainer.find('.body').blur();

    let hour = $cardContainer.find('.hour').val();
    if (hour.length == 0) {
      hour = '00';
    } else if (hour.length == 1) {
      hour = '0' + hour;
    }
    $cardContainer.find('.hour').val(hour);

    let minute = $cardContainer.find('.minute').val();
    if (minute.length == 0) {
      minute = '00';
    } else if (minute.length == 1) {
      minute = '0' + minute;
    }
    $cardContainer.find('.minute').val(minute);

    let second = $cardContainer.find('.second').val();
    if (second.length == 0) {
      second = '00';
    } else if (second.length == 1) {
      second = '0' + second;
    }
    $cardContainer.find('.second').val(second);

    let formattedInput = hour + ':' + minute + ':' + second;

    this.controller.onTimeChange(formattedInput);
  }

  /** @private */
  onCardClick(event) {
    if (!this.componentModel.isReadOnly) {
      if (this.selected) {
        this.selected = false;
        this.onDeselected();
      } else {
        this.selected = true;
        this.onSelected();
      }
    }
  }

  render() {
    let transitionTime = timeFormatter.formatDuration(this.componentModel.value);

    return (
      <div id={this.componentModel.componentId} className="card-container time-card-container">
        <div className="title">{this.componentModel.title}</div>
        <div className="body">
          <input className="hour" type="text" size="2" maxLength="2" defaultValue="00"/>:
          <input className="minute" type="text" size="2" maxLength="2" defaultValue="00"/>:
          <input className="second" type="text" size="2" maxLength="2" defaultValue="00"/>
        </div>
      </div>
    );
  }
}

export default View;
