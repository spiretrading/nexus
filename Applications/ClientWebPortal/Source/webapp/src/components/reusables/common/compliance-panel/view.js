import './style.scss';
import moment from 'moment';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import Dropdown from 'components/reusables/common/dropdown';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import inputValidator from 'utils/input-validator';
import deviceDetector from 'utils/device-detector';
import labelFormatter from 'utils/label-formatter';
import {DataType, Security} from 'spire-client';
import PrimaryButton from 'components/reusables/common/primary-button';
import modal from 'utils/modal';
import SymbolsModel from './symbols-model';
import HashMap from 'hashmap';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isInitialized = false;
    this.isInputFocused = false;
    this.symbolsModel = new SymbolsModel();

    this.onSymbolsInputClick = this.onSymbolsInputClick.bind(this);
    this.getCurrencyInput = this.getCurrencyInput.bind(this);
    this.getMoneyInput = this.getMoneyInput.bind(this);
    this.getSymbolsInput = this.getSymbolsInput.bind(this);
    this.getPeriodInput = this.getPeriodInput.bind(this);
    this.getIntegerInput = this.getIntegerInput.bind(this);
    this.getBooleanInput = this.getBooleanInput.bind(this);
    this.onModalCloseClick = this.onModalCloseClick.bind(this);
    this.onSymbolsModalItemClick = this.onSymbolsModalItemClick.bind(this);
    this.onKeyDownCheckSelectionModifiers = this.onKeyDownCheckSelectionModifiers.bind(this);
    this.onKeyUpCheckSelectionModifiers = this.onKeyUpCheckSelectionModifiers.bind(this);
    this.onSymbolsModalSaveClick = this.onSymbolsModalSaveClick.bind(this);
    this.onSymbolsInput = this.onSymbolsInput.bind(this);
    this.onDocumentMouseUp = this.onDocumentMouseUp.bind(this);
    this.onSymbolsSearchResultClick = this.onSymbolsSearchResultClick.bind(this);
    this.onAddSymbolClick = this.onAddSymbolClick.bind(this);
    this.onRemoveSymbolClick = this.onRemoveSymbolClick.bind(this);
    this.onImportSymbolsClick = this.onImportSymbolsClick.bind(this);
    this.onImportSymbolsFileSelect = this.onImportSymbolsFileSelect.bind(this);
  }

  /** @private */
  onExpandClick(event) {
    let $contentWrapper, $contentSlideWrapper;
    if (deviceDetector.isMobile()) {
      $contentWrapper = $(event.currentTarget).parent().parent().parent().find('.content-wrapper');
      $contentSlideWrapper = $(event.currentTarget).parent().parent().parent().find('.content-slide-wrapper');
    } else {
      $contentWrapper = $(event.currentTarget).parent().parent().find('.content-wrapper');
      $contentSlideWrapper = $(event.currentTarget).parent().parent().find('.content-slide-wrapper');
    }

    let contentHeight = $contentWrapper.height() + 20;
    let contentSlideWrapperHeight = $contentSlideWrapper.height();
    if (contentSlideWrapperHeight == 0) {
      $contentSlideWrapper.stop().animate({
        height: contentHeight
      });
      $(event.currentTarget).removeClass('collapsed').addClass('expanded')
        .parent().parent().removeClass('collapsed').addClass('expanded');
    } else {
      $contentSlideWrapper.stop().animate({
        height: 0
      });
      $(event.currentTarget).removeClass('expanded').addClass('collapsed')
        .parent().parent().removeClass('expanded').addClass('collapsed');;
    }
  }

  /** @private */
  adjustContentSlideWrapperHeight() {
    let $contentWrapper = $('#' + this.componentModel.componentId + ' .content-wrapper');
    let $contentSlideWrapper = $('#' + this.componentModel.componentId + ' .content-slide-wrapper');
    let contentHeight = $contentWrapper.height() + 20;
    $contentSlideWrapper.height(contentHeight);
  }

  /** @private */
  getParameter(parameterName) {
    let parameters = this.componentModel.parameters;
    for (let i=0; i<parameters.length; i++) {
      if (parameters[i].name === parameterName) {
        return parameters[i];
      }
    }
  }

  /** @private */
  onMoneyInputBlur(event) {
    let $input = $(event.currentTarget);
    let input = $input.val().trim();
    let formattedNumber = numberFormatter.formatTwoDecimalsWithComma(input);
    $input.val(formattedNumber);
    let value = formattedNumber.replace(new RegExp(',', 'g'), '');

    // validate input
    let valueNumber = Number(value);
    let errorMessage;
    if (!$.isNumeric(valueNumber)) {
      errorMessage = 'Must be a number.';
    } else if (valueNumber < 0) {
      errorMessage = 'Cannot be a negative number.';
    }

    let $inputWrapper = $input.parent();
    if (errorMessage != null) {
      $inputWrapper.find('.validation-error').html(errorMessage).css('display', 'inherit');

      if (!$input.hasClass('invalid-input')) {
        this.showValidationErrorMessage($input, errorMessage);
      }
    } else {
      let parameterName = $(event.currentTarget).attr('data-parameter-name');
      this.controller.onParameterUpdated(parameterName, value);

      if ($input.hasClass('invalid-input')) {
        this.hideValidationErrorMessage($input);
      }
    }
  }

  /** @private */
  onSymbolsInputClick(e) {
    let parameterIndex = $(e.currentTarget).attr('data-parameter-name');
    this.openSymbolsModal();
  }

  /** @private */
  onIntegerInputBlur(event) {
    let $input = $(event.currentTarget);
    let input = $input.val().trim();
    let formattedNumber = numberFormatter.formatWithComma(input);
    $input.val(formattedNumber);

    let value = formattedNumber.replace(',', '');
    let parameterName = $(event.currentTarget).attr('data-parameter-name');
    this.controller.onParameterUpdated(parameterName, value);
  }

  /** @private */
  onBooleanInputChange(event) {
    let $checkbox = $(event.currentTarget);
    let parameterName = $checkbox.attr('data-parameter-name');
    let isChecked = $checkbox.prop('checked');
    this.controller.onParameterUpdated(parameterName, isChecked);
  }

  /** @private */
  onStatusChange(newValue) {
    this.controller.onStatusChange(newValue, this.componentModel.schema.parameters);
  }

  /** @private */
  onTimeChange(e) {
    let $timeWrapper = $(e.currentTarget).parent();
    let hours = $timeWrapper.find('.hour').val();
    if (hours.length == 1) {
      hours = '0' + hours;
    }
    let minutes = $timeWrapper.find('.minute').val();
    if (minutes.length == 1) {
      minutes = '0' + minutes;
    }
    let seconds = $timeWrapper.find('.second').val();
    if (seconds.length == 1) {
      seconds = '0' + seconds;
    }

    // validate input
    let errorMessage;
    if (Number(hours) > 23) {
      errorMessage = 'Hours must be less than 24.';
    } else if (Number(minutes) > 59) {
      errorMessage = 'Minutes must be less than 60.';
    } else if (Number(seconds) > 59) {
      errorMessage = 'Seconds must be less than 60.';
    }

    if (errorMessage != null) {
      $timeWrapper.parent().find('.validation-error').html(errorMessage).css('display', 'inherit');

      if (!$timeWrapper.hasClass('invalid-input')) {
        this.showValidationErrorMessage($timeWrapper, errorMessage);
      }
    } else {
      let timeInput = hours + ':' + minutes + ':' + seconds;
      let parameterName = $timeWrapper.attr('data-parameter-name');

      timeInput = this.toUtcTime(timeInput);
      this.controller.onParameterUpdated(parameterName, timeInput);

      if ($timeWrapper.hasClass('invalid-input')) {
        this.hideValidationErrorMessage($timeWrapper);
      }
    }
  }

  /** @private */
  toUtcTime(timeInput) {
    timeInput = timeInput.split(':');
    let localNow = moment();
    localNow.set({
      hour: Number(timeInput[0]),
      minute: Number(timeInput[1]),
      second: Number(timeInput[2])
    });
    return localNow.utc().format('HH:mm:ss');
  }

  /** @private */
  showValidationErrorMessage($input, errorMessage) {
    $input.addClass('invalid-input');
    let $contentSlideWrapper = $('#' + this.componentModel.componentId + ' .content-slide-wrapper');
    let contentSlideWrapperHeight = $contentSlideWrapper.height();
    $contentSlideWrapper.height(contentSlideWrapperHeight + 15);
  }

  /** @private */
  hideValidationErrorMessage($input) {
    $input.removeClass('invalid-input');
    $input.parent().find('.validation-error').html('').css('display', 'none');
    let $contentSlideWrapper = $('#' + this.componentModel.componentId + ' .content-slide-wrapper');
    let contentSlideWrapperHeight = $contentSlideWrapper.height();
    $contentSlideWrapper.height(contentSlideWrapperHeight - 15);
  }

  /** @private */
  getCurrencyInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let currencyCodes = definitionsService.getAllCurrencyCodes();
    let currencyOptions = [];
    for (let i=0; i<currencyCodes.length; i++) {
      let currencyNumber = definitionsService.getCurrencyNumber(currencyCodes[i]);
      currencyOptions.push({
        label: currencyCodes[i],
        value: currencyNumber
      });
    }

    let currencyDropdownModel = {
      selectedValue: parameters[parameterIndex].value.value,
      options: currencyOptions,
      isDisabled: !this.componentModel.isAdmin
    };
    let onCurrencyChange = (newValue) => {
      this.controller.onParameterUpdated(parameters[parameterIndex].name, newValue);
    };
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value currency-value">
          <Dropdown className="currency-dropdown" model={currencyDropdownModel} onChange={onCurrencyChange}/>
        </div>
      </div>;

    return input;
  }

  /** @private */
  getMoneyInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let formattedNumber = numberFormatter.formatTwoDecimalsWithComma(parameters[parameterIndex].value.value.toNumber());
    let onMoneyInputBlur = this.onMoneyInputBlur.bind(this);
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value money-value">
          <input type="text" className="money-input"
                 data-parameter-name={parameters[parameterIndex].name}
                 onBlur={onMoneyInputBlur}
                 defaultValue={formattedNumber}
                 readOnly={!this.componentModel.isAdmin}/>
          <div className="validation-error"></div>
        </div>
      </div>;

    return input;
  }

  /** @private */
  getSymbolsInput(parameterIndex) {
    let marketDatabase = definitionsService.getMarketDatabase();
    let parameters = this.componentModel.schema.parameters;
    let symbols = '';
    let sortedSymbols = this.getSortedSymbols(parameters[parameterIndex].value.value);
    for (let j=0; j<sortedSymbols.length; j++) {
      let security = sortedSymbols[j].value;
      if (security.symbol.length > 0) {
        let symbolLabel = security.toString(marketDatabase);
        symbols += symbolLabel + ', ';
      }
    }
    symbols = symbols.substring(0, symbols.length - 2);
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value">
          <input type="text"
            className="symbols-input"
            value={symbols}
            readOnly
            onClick={this.onSymbolsInputClick}
            data-parameter-name={parameters[parameterIndex].name}
          />
        </div>
      </div>;

    return input;
  }

  /** @private */
  getSortedSymbols(originalSymbols) {
    let marketDatabase = definitionsService.getMarketDatabase();
    let sorted = originalSymbols.slice();
    sorted.sort((a,b) => {
      let securityA = a.value;
      let securityB = b.value;
      return securityA.toString(marketDatabase).localeCompare(securityB.toString(marketDatabase));
    });
    return sorted;
  }

  /** @private */
  getPeriodInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);
    let timeValue = parameters[parameterIndex].value.value;
    timeValue = this.toLocalTime(timeValue);
    let timeValues = timeValue.split(':');
    let hour = timeValues[0];
    let minute = timeValues[1];
    let second = timeValues[2];

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value">
          <div className="time-input-wrapper" data-parameter-name={parameters[parameterIndex].name}>
            <input className="hour numeric"
                   type="text" size="2"
                   maxLength="2"
                   defaultValue={hour}
                   onBlur={this.onTimeChange.bind(this)}
                   readOnly={!this.componentModel.isAdmin}/>:
            <input className="minute numeric"
                   type="text" size="2"
                   maxLength="2"
                   defaultValue={minute}
                   onBlur={this.onTimeChange.bind(this)}
                   readOnly={!this.componentModel.isAdmin}/>:
            <input className="second numeric"
                   type="text"
                   size="2"
                   maxLength="2"
                   defaultValue={second}
                   onBlur={this.onTimeChange.bind(this)}
                   readOnly={!this.componentModel.isAdmin}/>
          </div>
          <div className="validation-error"></div>
        </div>
      </div>;

    return input;
  }

  /** @private */
  toLocalTime(time) {
    let utcDate = moment.utc().format('YYYY-MM-DD');
    let utcOffset = (new Date().getTimezoneOffset()) * -1;
    let localTime = moment.utc(utcDate + 'T' + time, "YYYY-MM-DDTHH:mm:ss").utcOffset(utcOffset);
    return localTime.format('HH:mm:ss');
  }

  /** @private */
  getIntegerInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let formattedNumber = numberFormatter.formatWithComma(parameters[parameterIndex].value.value);
    let onIntegerInputBlur = this.onIntegerInputBlur.bind(this);
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value count-value">
          <input type="text" className="count-input"
                 data-parameter-name={parameters[parameterIndex].name}
                 onBlur={onIntegerInputBlur}
                 defaultValue={formattedNumber}
                 readOnly={!this.componentModel.isAdmin}/>
        </div>
      </div>;

    return input;
  }

  /** @private */
  getBooleanInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);
    let isTrue = parameters[parameterIndex].value.value;
    let onBooleanInputChange = this.onBooleanInputChange.bind(this);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value boolean-input">
          <input type="checkbox"
                 data-parameter-name={parameters[parameterIndex].name}
                 defaultChecked={isTrue}
                 onChange={onBooleanInputChange}
                 readOnly={!this.componentModel.isAdmin}/>
        </div>
      </div>;

    return input;
  }

  initialize() {
    inputValidator.onlyNumbers($('#' + this.componentModel.componentId + ' .buying-power-input'));
    inputValidator.onlyNumbers($('#' + this.componentModel.componentId + ' .time-input-wrapper input.numeric'));

    $(document).keydown(this.onKeyDownCheckSelectionModifiers)
      .keyup(this.onKeyUpCheckSelectionModifiers);

    var _this = this;
    let $symbolsList = $('#' + this.componentModel.componentId + ' .symbols-modal .symbols-list');
    if ($symbolsList[0] != null) {
      let height = $symbolsList.height();
      let scrollHeight = $symbolsList.get(0).scrollHeight;
      $symbolsList.bind('mousewheel', function(e, d) {
        e.stopPropagation();
      });
      $('#' + this.componentModel.componentId + ' .symbols-modal .autocomplete-wrapper').bind('mousewheel', function(e) {
        e.stopPropagation();
      });
    }

    this.isInitialized = true;

    $('#' + this.componentModel.componentId + ' .boolean-input input').prop('checked', true);

    $('#' + this.componentModel.componentId + ' .symbols-input input').focus(function() {
      if (!_this.isInputFocused) {
        _this.isInputFocused = true;
        $(this).parent().parent().addClass('p1-solid-border');
      }
    }).blur(function() {
      _this.isInputFocused = false;
      $(this).parent().parent().removeClass('p1-solid-border');
    });

    $('#' + this.componentModel.componentId + ' .time-input-wrapper input').focus(function() {
      if (!_this.isInputFocused) {
        _this.isInputFocused = true;
        $(this).parent().addClass('p1-solid-border');
      }
    }).blur(function() {
      _this.isInputFocused = false;
      $(this).parent().removeClass('p1-solid-border');
    });
  }

  dispose() {
    $(document).unbind('keydown', this.onKeyDownCheckSelectionModifiers)
      .unbind('keyup', this.onKeyUpCheckSelectionModifiers);
    $(document).unbind('mouseup', this.onDocumentMouseUp);
  }

  /** @private */
  onDocumentMouseUp(e) {
    let $autocompleteWrapper = $('#' + this.componentModel.componentId + ' .autocomplete-wrapper');
    let $parent = $(e.target).parent();
    if ($autocompleteWrapper[0] !== $parent[0]) {
      this.closeSymbolsSearchResults();
    }
  }

  /** @private */
  closeSymbolsSearchResults() {
    this.symbolsSearchResults = null;
    this.update();
  }

  /** @private */
  onSymbolsModalItemClick(e) {
    let $row = $(e.currentTarget);
    let clickedRowIndex = Number($row.attr('data-row-index'));

    if (!this.isCntrlPressed && !this.isShiftPressed) {
      // when no modifiers are pressed
      this.activeRowIndex = clickedRowIndex;
      this.symbolsModel.clearSelectedRows();
      this.symbolsModel.setSelectedRow(clickedRowIndex, false);
    } else if (this.isCntrlPressed && !this.isShiftPressed) {
      // when only the control modifier is pressed
      this.activeRowIndex = clickedRowIndex;
      if (this.symbolsModel.isRowSelectedControlModified(clickedRowIndex)) {
       this.symbolsModel.removeRowSelection(clickedRowIndex);
      } else {
        this.symbolsModel.setSelectedRow(clickedRowIndex, true);

        // set all existing selected rows with control modifier applied
        this.symbolsModel.setAllSelectedRowsControlModified();
      }
    } else {
      // when shift modifier is pressed
      let $allRows = $row.parent().children();
      let parameters = this.componentModel.schema.parameters;
      let rowCounter = parameters[this.symbolsParametersIndex].value.value.length;
      let pivotRowsMetCounter = 0;

      // unselect the selected rows not chosen by control modifier
      this.symbolsModel.removeNonControlModifiedSelectedRows();

      for (let i=0; i<rowCounter; i++) {
        if (this.activeRowIndex == null) {
          this.activeRowIndex = clickedRowIndex;
        }

        let $currentRow = $(this);
        let currentRowIndex = i;
        if (currentRowIndex === this.activeRowIndex || currentRowIndex === clickedRowIndex) {
          pivotRowsMetCounter++;
        }

        if (pivotRowsMetCounter > 0 && pivotRowsMetCounter <= 2) {
          this.symbolsModel.setSelectedRow(currentRowIndex, false);

          if (pivotRowsMetCounter == 2) {
            pivotRowsMetCounter++;
          }
        }
      }
    }
    this.update();
  }

  /** @private */
  getSymbolsModal(parameterIndex) {
    this.symbolsParametersIndex = parameterIndex;
    let symbols = [];
    let marketDatabase = definitionsService.getMarketDatabase();
    let parameters = this.componentModel.schema.parameters;
    let sortedSymbols = this.getSortedSymbols(parameters[parameterIndex].value.value);
    let selectedRows = this.symbolsModel.getSelectedRows();
    for (let j=0; j<sortedSymbols.length; j++) {
      let security = sortedSymbols[j].value;
      if (security.symbol.length > 0) {
        let className = 'no-select';
        if (selectedRows.includes(j)) {
          className += ' selected';
        }
        symbols.push(
          <li key={j}
            className={className}
            onClick={this.onSymbolsModalItemClick}
            data-row-index={j}
            data-market-code={security.market.value}
            data-security-symbol={security.symbol}
          >
              {security.toString(marketDatabase)}
          </li>
        );
      }
    }

    let searchResultsClassName = 'autocomplete-wrapper';
    let searchResults;
    if (this.symbolsSearchResults == null) {
      searchResultsClassName += ' hidden';
    } else {
      searchResults = [];
      for (let i=0; i<this.symbolsSearchResults.length; i++) {
        searchResults.push(
          <li key={i} onClick={this.onSymbolsSearchResultClick}>
            {this.symbolsSearchResults[i].security.toString(marketDatabase)}
          </li>
        );
      }
    }

    return  <div className="modal fade symbols-modal" tabIndex="-1" role="dialog">
              <div className="modal-dialog" role="document">
                <div className="modal-content">
                  <div className="modal-header">
                    Edit Symbols
                    <span className="icon-close" onClick={this.onModalCloseClick}></span>
                  </div>
                  <div className="modal-body">
                    <div className="symbol-input-container">
                      <input
                        className="symbol-input"
                        placeholder="Type symbol here..."
                        onInput={this.onSymbolsInput}
                      />
                      <ul className={searchResultsClassName}>
                        {searchResults}
                      </ul>
                    </div>
                    <div className="action-buttons-container">
                      <div className="Remove" onClick={this.onRemoveSymbolClick}>Remove</div>
                      <div className="import" onClick={this.onImportSymbolsClick}>Import</div>
                      <input type="file" className="import-input"/>
                      <div className="add" onClick={this.onAddSymbolClick}>Add</div>
                    </div>
                    <ul className="symbols-list" data-parameter-name={parameters[parameterIndex].name}>
                      {symbols}
                    </ul>
                    <div className="buttons">
                      <PrimaryButton 
                        className="save-changes-button button" 
                        model={{label: 'Save Changes'}}
                        onClick={this.onSymbolsModalSaveClick}
                      />
                    </div>
                  </div>
                </div>
              </div>
            </div>;
  }

  /** @private */
  onAddSymbolClick() {
    let $symbolInput = $('#' + this.componentModel.componentId + ' .symbols-modal .symbol-input');
    let securityDisplay = $symbolInput.val().trim();
    this.addNonExistentSymbol(securityDisplay);
    this.update();
    $symbolInput.val('');
  }

  /** @private */
  addNonExistentSymbol(securityDisplay) {
    let $includingElements = $("#" + this.componentModel.componentId + " .symbols-modal li:contains('" + securityDisplay + "')");
    let doesExist = $includingElements.filter(function() {
      return $(this).text() === securityDisplay;
    }).length > 0;
    if (!doesExist) {
      let marketDatabase = definitionsService.getMarketDatabase();
      let security = Security.fromDisplay(securityDisplay, marketDatabase);
      this.componentModel.schema.parameters[this.symbolsParametersIndex].value.value.push({
        value: security
      });
    }
  }

  /** @private */
  onRemoveSymbolClick() {
    let selectedIndices = this.symbolsModel.getSelectedRows();
    let selectedSecurityDisplays = new HashMap();
    let symbolsElements = $("#" + this.componentModel.componentId + " .symbols-modal li");
    for (let i=0; i<selectedIndices.length; i++) {
      let $selectedElement = $(symbolsElements.get(selectedIndices[i]));
      selectedSecurityDisplays.set($selectedElement.text(), true);
    }
    let securities = this.componentModel.schema.parameters[this.symbolsParametersIndex].value.value;
    let marketDatabase = definitionsService.getMarketDatabase();
    let filtered = securities.filter(security => {
      let securityDisplay = security.value.toString(marketDatabase);
      if (selectedSecurityDisplays.has(securityDisplay)) {
        return false;
      } else {
        return true;
      }
    });
    this.componentModel.schema.parameters[this.symbolsParametersIndex].value.value = filtered;
    this.symbolsModel.clearSelectedRows();
    this.update();
  }

  /** @private */
  onImportSymbolsClick() {
    $('#' + this.componentModel.componentId + ' .symbols-modal .import-input')
      .change(this.onImportSymbolsFileSelect)
      .trigger('click');
  }

  /** @private */
  onImportSymbolsFileSelect(e) {
    $('#' + this.componentModel.componentId + ' .symbols-modal .import-input')
      .unbind('change', this.onImportSymbolsFileSelect);
    let selectedFile = e.currentTarget.files[0];
    if (selectedFile) {
      let reader = new FileReader();
      reader.readAsText(selectedFile, 'UTF-8');
      reader.onload = (e) => {
        let fileContent = e.target.result;
        let symbols = fileContent.split(/\r?\n/);
        for (let i=0; i<symbols.length; i++) {
          if (symbols[i].trim().length > 0) {
            this.addNonExistentSymbol(symbols[i]);
          }
        }
        this.update();
      };
    }
  }

  /** @private */
  onSymbolsSearchResultClick(e) {
    let securityDisplay = $(e.currentTarget).text().trim();
    $('#' + this.componentModel.componentId + ' .symbols-modal .symbol-input').val(securityDisplay);
    this.closeSymbolsSearchResults();
  }

  /** @private */
  onSymbolsInput(e) {
    if (this.symbolsInputTimeout != null) {
      clearTimeout(this.symbolsInputTimeout);
    }

    let inputElement = e.currentTarget;
    let marketDatabase = definitionsService.getMarketDatabase();
    this.symbolsInputTimeout = setTimeout(() => {
      this.symbolsInputTimeout = null;
      let input = $(inputElement).val().trim();
      if (input.length > 0) {
        this.controller.searchSymbols(input)
          .then((results) => {
            if (results.length > 0) {
              this.symbolsSearchResults = results;
            }
            this.update();
          });
      } else {
        this.symbolsSearchResults = null;
        this.update();
      }
    }, Config.INPUT_TIMEOUT_DURATION);
  }

  /** @private */
  onSymbolsModalSaveClick() {
    let securities = [];
    let $symbols = $('#' + this.componentModel.componentId + ' .symbols-list li');
    $symbols.each(function() {
      let marketCode = $(this).attr('data-market-code');
      let securitySymbol = $(this).attr('data-security-symbol');
      let label = $(this).text();
      let security;
      if (Security.isWildCard(label)) {
        security = Security.getWildCard();
      } else {
        let labelTokens = label.split('.');
        let symbol = labelTokens[0];
        let country = definitionsService.getMarket(marketCode).countryCode;
        security = Security.fromData({
          country: country.toNumber(),
          market: marketCode,
          symbol: securitySymbol
        });
      }
      let securityDefinition = {
        value: security,
        which: 8
      };
      securities.push(securityDefinition);
    });
    let parameterName = $('#' + this.componentModel.componentId + ' .symbols-list').attr('data-parameter-name');
    this.controller.onParameterUpdated(parameterName, securities);
    this.symbolsModel.clearSelectedRows();
    $('#' + this.componentModel.componentId + ' .symbol-input').val('');
    this.update();
    this.closeSymbolsModal();
  }

  /** @private */
  onKeyDownCheckSelectionModifiers(event) {
    let isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;
    if((!isMac && event.which == '17') || (isMac && event.which == '91')) {
      this.isCntrlPressed = true;
    } else if (event.which == '16') {
      this.isShiftPressed = true;
    }
  }

  /** @private */
  onKeyUpCheckSelectionModifiers(event) {
    let isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;
    if((!isMac && event.which == '17') || (isMac && event.which == '91')) {
      this.isCntrlPressed = false;
    } else if (event.which == '16') {
      this.isShiftPressed = false;
    }
  }

  /** @private */
  openSymbolsModal() {
    modal.show($('#' + this.componentModel.componentId + ' .symbols-modal'));
    $(document).mouseup(this.onDocumentMouseUp);
  }

  /** @private */
  closeSymbolsModal() {
    modal.hide($('#' + this.componentModel.componentId + ' .symbols-modal'));
    $(document).unbind('mouseup', this.onDocumentMouseUp);
  }

  onModalCloseClick() {
    modal.hide($('#' + this.componentModel.componentId + ' .symbols-modal'));
  }

  render() {
    let name = this.componentModel.schema.name;

    let stateDropdownModel;
    if (this.componentModel.isGroup) {
      stateDropdownModel = {
        selectedValue: this.componentModel.state,
        isDisabled: !this.componentModel.isAdmin,
        options: [
          {
            label: 'Consolidated Active',
            value: 0
          },
          {
            label: 'Consolidated Passive',
            value: 1
          },
          {
            label: 'Per Account Active',
            value: 2
          },
          {
            label: 'Per Account Passive',
            value: 3
          },
          {
            label: 'Disabled',
            value: 4
          },
          {
            label: 'Delete',
            value: 5
          },
        ]
      };
    } else {
      stateDropdownModel = {
        selectedValue: this.componentModel.state,
        isDisabled: !this.componentModel.isAdmin,
        options: [
          {
            label: 'Active',
            value: 0
          },
          {
            label: 'Passive',
            value: 1
          },
          {
            label: 'Disabled',
            value: 2
          },
          {
            label: 'Delete',
            value: 3
          },
        ]
      };
    }

    let onStatusChange = this.onStatusChange.bind(this);
    let content = [];
    let symbolsModal = null;
    let parameters = this.componentModel.schema.parameters;
    let schema = definitionsService.getComplianceRuleScehma(this.componentModel.schema.name);
    let schemaParameters = schema.parameters;
    for (let i=0; i<parameters.length; i++) {
      if (schemaParameters[i].value.which == DataType.CURRENCY) {
        content.push(this.getCurrencyInput(i));
      } else if (schemaParameters[i].value.which == DataType.MONEY) {
        content.push(this.getMoneyInput(i));
      } else if (schemaParameters[i].value.which == DataType.LIST && schemaParameters[i].value.value[0].which == DataType.SECURITY) {
        content.push(this.getSymbolsInput(i));
        symbolsModal = this.getSymbolsModal(i);
      } else if (schemaParameters[i].value.which == DataType.TIME_DURATION) {
        content.push(this.getPeriodInput(i));
      } else if (schemaParameters[i].value.which == DataType.INT64) {
        content.push(this.getIntegerInput(i));
      } else if (schemaParameters[i].value.which == DataType.BOOLEAN) {
        content.push(this.getBooleanInput(i));
      }
    }

    let header;
    if (deviceDetector.isMobile()) {
      header =
        <div className="header">
          <div className="title-wrapper">
            <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
            <span className="title">{labelFormatter.toCapitalWithSpace(name)}</span>
          </div>
          <Dropdown className="status-dropdown" model={stateDropdownModel} onChange={onStatusChange}/>
        </div>;
    } else {
      header =
        <div className="header">
          <span className="icon-expand" onClick={this.onExpandClick.bind(this)}></span>
          <span className="title">{labelFormatter.toCapitalWithSpace(name)}</span>
          <Dropdown className="status-dropdown" model={stateDropdownModel} onChange={onStatusChange}/>
        </div>;
    }

    return (
      <div id={this.componentModel.componentId} className="compliance-panel-container">
        {header}
        <div className="content-slide-wrapper">
          <div className="content-wrapper">
            <div className="content-header">
              Parameters
            </div>
            <div className="content-body">
              {content}
            </div>
          </div>
        </div>
        {symbolsModal}
      </div>
    );
  }
}

export default View;
