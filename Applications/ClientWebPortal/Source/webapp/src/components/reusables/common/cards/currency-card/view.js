import '../style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import currencyFormatter from 'utils/currency-formatter';
import definitionsService from 'services/definitions';
import contentEditableCaret from 'utils/content-editable-caret';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selected = false;
    this.isTouched = false;
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
    this.isTouched = false;
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
    $cardContainer.find('.body').removeAttr('contenteditable').blur();

    let currentInput = $cardContainer.find('.body').text().trim();
    let currencySign = definitionsService.getCurrencySign.apply(
      definitionsService,
      [this.componentModel.countryIso]
    );
    let regexToReplace = new RegExp('[ ,' + currencySign + ']', 'gi');
    currentInput = currentInput.replace(regexToReplace, '');
    currentInput = parseFloat(currentInput);
    let formattedInput = currencyFormatter.format(this.componentModel.countryIso, currentInput);
    $cardContainer.find('.body').text(formattedInput);

    this.controller.onAmountChange(currentInput);
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
    } else {
      if (!isLessThanTwoDecimals.apply(this)) {
        event.preventDefault();
      } else {
        this.isTouched = true;
      }
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
        event.keyCode === 8           // backspace
        || event.keyCode === 46       // delete
        || event.keyCode === 188      // comma
        || event.keyCode === 37       // arrow left
        || event.keyCode === 39       // arrow right
        || event.keyCode === 35       // end
        || event.keyCode === 36       // home
        || event.keyCode === 190      // period
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

    function isLessThanTwoDecimals() {
      let $cardContainer = $('#' + this.componentModel.componentId);
      let bodyElement = $cardContainer.find('.body')[0];
      let caretPosition = contentEditableCaret.getCaretPosition(bodyElement);
      let offsetFromEnd;
      if (!this.isTouched) {
        offsetFromEnd = 0;
      } else {
        offsetFromEnd = $(bodyElement).text().trim().length - caretPosition;
      }
      let currentInput = event.target.innerText.trim();
      let periodIndex = currentInput.indexOf('.');
      if (periodIndex >= 0) {
        let decimals = currentInput.substring(periodIndex + 1);
        if (decimals.length == 2 && offsetFromEnd === 0 && isNumber()) {
          return false;
        } else {
          return true;
        }
      } else {
        return true;
      }
    }
  }

  render() {
    let amount = currencyFormatter.format(this.componentModel.countryIso, this.componentModel.value);

    return (
      <div id={this.componentModel.componentId} className="card-container" onClick={this.onCardClick.bind(this)}>
        <div className="title">{this.componentModel.title}</div>
        <div className="body" onKeyDown={this.onChange.bind(this)}>{amount}</div>
      </div>
    );
  }
}

export default View;
