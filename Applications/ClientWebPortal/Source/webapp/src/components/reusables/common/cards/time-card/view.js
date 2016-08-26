import '../style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import timeFormatter from 'utils/time-formatter';
import contentEditableCaret from 'utils/content-editable-caret';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selected = false;
  }

  initialize() {
    if (!this.componentModel.isReadOnly) {
      $(document).on('click.' + this.componentModel.componentId, this.onDocumentClick.bind(this));
    }
  }

  dispose() {
    if (!this.componentModel.isReadOnly) {
      $(document).off('click.' + this.componentModel.componentId);
    }
  }

  onDocumentClick(event) {
    let cardContainerElement = $('#' + this.componentModel.componentId)[0];
    let clickedElement = event.target;
    let isCardClicked = $.contains(cardContainerElement, clickedElement) || cardContainerElement === clickedElement;

    if (!isCardClicked && this.selected) {
      this.selected = false;
      this.onDeselected();
    }
  }

  onSelected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.addClass('selected');
    $cardContainer.find('.body').attr('contenteditable', 'true').focus();
    contentEditableCaret.moveCaretToEnd($cardContainer.find('.body')[0]);
  }

  onDeselected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
    $cardContainer.find('.body').removeAttr('contenteditable').blur();

    let currentInput = $cardContainer.find('.body').text().trim();
    let formattedInput = timeFormatter.formatDuration(currentInput);
    $cardContainer.find('.body').text(formattedInput);

    this.controller.onTimeChange(formattedInput);
  }

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

  onChange(e) {
    let event = e.nativeEvent;
    if (isEnter()) {
      this.selected = false;
      this.onDeselected();
    } else if (!isNumber() && !isAllowedSpecialCharacters()) {
      event.preventDefault();
    } else if (isTooLong.apply(this) && !isAllowedNonAdditiveCharacters()) {
      event.preventDefault();
    }

    function isNumber() {
      if ((event.keyCode >= 48 && event.keyCode <= 57) || (event.keyCode >= 96 && event.keyCode <= 105)) {
        return true;
      } else {
        return false;
      }
    }

    function isAllowedSpecialCharacters() {
      if (
        event.keyCode === 186         // colon, semi-colon
        || event.keyCode === 8        // backspace
        || event.keyCode === 46       // delete
        || event.keyCode === 37       // arrow left
        || event.keyCode === 39       // arrow right
        || event.keyCode === 35       // end
        || event.keyCode === 36       // home
      ) {
        return true;
      } else {
        return false;
      }
    }

    function isAllowedNonAdditiveCharacters() {
      if (
        event.keyCode === 8           // backspace
        || event.keyCode === 46       // delete
        || event.keyCode === 37       // arrow left
        || event.keyCode === 39       // arrow right
        || event.keyCode === 35       // end
        || event.keyCode === 36       // home
      ) {
        return true;
      } else {
        return false;
      }
    }

    function isEnter() {
      if (event.keyCode === 13) {
        return true;
      } else {
        return false;
      }
    }

    function isTooLong() {
      let $cardContainer = $('#' + this.componentModel.componentId);
      let currentInput = $cardContainer.find('.body').text();
      if (currentInput.length === 8) {
        return true;
      } else {
        return false;
      }
    }
  }

  render() {
    let transitionTime = timeFormatter.formatDuration(this.componentModel.value);

    return (
      <div id={this.componentModel.componentId} className="card-container" onClick={this.onCardClick.bind(this)}>
        <div className="title">{this.componentModel.title}</div>
        <div className="body" onKeyDown={this.onChange.bind(this)}>{transitionTime}</div>
      </div>
    );
  }
}

export default View;
