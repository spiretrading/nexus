import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import Dropdown from 'components/reusables/common/dropdown';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import inputValidator from 'utils/input-validator';
import deviceDetector from 'utils/device-detector';
import labelFormatter from 'utils/label-formatter';
import {DataType, Security} from 'spire-client';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isInitialized = false;
    this.isInputFocused = false;
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
      $(event.currentTarget).removeClass('collapsed').addClass('expanded');
    } else {
      $contentSlideWrapper.stop().animate({
        height: 0
      });
      $(event.currentTarget).removeClass('expanded').addClass('collapsed');
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
    if (isNaN(valueNumber)) {
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
      this.controller.onParameterUpdated.apply(this.controller, [
        parameterName,
        value
      ]);

      if ($input.hasClass('invalid-input')) {
        this.hideValidationErrorMessage($input);
      }
    }
  }

  /** @private */
  onSymbolsChange() {
    let securities = [];
    let $tagsUl = $('#' + this.componentModel.componentId + ' ul.symbols-input');
    $tagsUl.find('.tagit-label').each(function() {
      let label = $(this).text();
      let security;
      if (Security.isWildCard(label)) {
        security = Security.getWildCard();
      } else {
        let labelTokens = label.split('.');
        let symbol = labelTokens[0];
        let market = labelTokens[1];
        let country = definitionsService.getMarket(market).country_code;
        security = Security.fromData({
          country: country,
          market: market,
          symbol: symbol
        });
      }

      let securityDefinition = {
        value: security,
        which: 8
      };
      securities.push(securityDefinition);
    });
    let parameterName = $tagsUl.attr('data-parameter-name');
    this.controller.onParameterUpdated(parameterName, securities);
  }

  /** @private */
  onIntegerInputBlur(event) {
    let $input = $(event.currentTarget);
    let input = $input.val().trim();
    let formattedNumber = numberFormatter.formatWithComma(input);
    $input.val(formattedNumber);

    let value = formattedNumber.replace(',', '');
    let parameterName = $(event.currentTarget).attr('data-parameter-name');
    this.controller.onParameterUpdated.apply(this.controller, [
      parameterName,
      value
    ]);
  }

  /** @private */
  onBooleanInputChange(event) {
    let $checkbox = $(event.currentTarget);
    let parameterName = $checkbox.attr('data-parameter-name');
    let isChecked = $checkbox.prop('checked');
    this.controller.onParameterUpdated.apply(this.controller, [
      parameterName,
      isChecked
    ]);
  }

  /** @private */
  onStatusChange(newValue) {
    this.controller.onStatusChange.apply(this.controller, [newValue, this.componentModel.schema.parameters]);
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

      this.controller.onParameterUpdated.apply(this.controller, [parameterName, timeInput]);

      if ($timeWrapper.hasClass('invalid-input')) {
        this.hideValidationErrorMessage($timeWrapper);
      }
    }
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
      this.controller.onParameterUpdated.apply(this.controller, [
        parameters[parameterIndex].name,
        newValue
      ]);
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
    let parameters = this.componentModel.schema.parameters;
    let tags = [];
    for (let j=0; j<parameters[parameterIndex].value.value.length; j++) {
      let security = parameters[parameterIndex].value.value[j].value;
      if (security.symbol.length > 0) {
        let tagLabel = security.symbol;
        if (security.market.toCode().length > 0) {
          tagLabel += '.' + security.market.toCode();
        }
        tags.push(
          <li key={j} data-country-code={security.country} data-symbol={security.symbol}>{tagLabel}</li>
        );
      }
    }
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value">
          <ul className="symbols-input" data-parameter-name={parameters[parameterIndex].name}>
            {tags}
          </ul>
        </div>
      </div>;

    return input;
  }

  /** @private */
  getPeriodInput(parameterIndex) {
    let parameters = this.componentModel.schema.parameters;
    let parameterName = parameters[parameterIndex].name.replace(/\\/g, '');
    parameterName = labelFormatter.toCapitalWithSpace(parameterName);
    let timeValues = parameters[parameterIndex].value.value.split(':');
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

    var _this = this;
    $('#' + this.componentModel.componentId + ' .symbols-input').each(function(){
      let $tagContainer = $(this);
      let sourceLabels = [];
      $tagContainer.tagit({
        autocomplete: {
          delay: 0,
          minLength: 1,
          source: function(request, response) {
            if (_this.symbolsTimeout != null) {
              clearTimeout(_this.symbolsTimeout);
            }

            _this.symbolsTimeout = setTimeout(() => {
              _this.symbolsTimeout = null;
              let input = $('#' + _this.componentModel.componentId + ' .ui-autocomplete-input').val().trim();
              _this.controller.searchSymbols(input)
                .then((results) => {
                  let labels = [];
                  for (let i=0; i<results.length; i++) {
                    let symbol = results[i].security.symbol + '.' + results[i].security.market;
                    let label = {
                      label: symbol + ' (' + results[i].name + ')',
                      value: symbol
                    };
                    labels.push(label);
                    if (sourceLabels.indexOf(label.value) < 0) {
                      sourceLabels.push(label.value);
                    }
                  }
                  response(labels);
                });
            }, Config.INPUT_TIMEOUT_DURATION);
          }
        },
        beforeTagAdded: function(event, ui) {
          if (_this.isInitialized) {
            if (ui.tagLabel != '*' && !doesExistInSourceLabels(ui.tagLabel)){
              $(this).find('.ui-autocomplete-input').val('');
              return false;
            }
          }

          function doesExistInSourceLabels(label) {
            for (let i=0; i<sourceLabels.length; i++) {
              if (sourceLabels[i] == label) {
                return true;
              }
            }
            return false;
          }
        },
        afterTagAdded: (event, ui) => {
          if (_this.isInitialized) {
            _this.adjustContentSlideWrapperHeight();
            _this.onSymbolsChange.apply(_this);
          } else {
            $(event.currentTarget).parent().parent().find('.content-slide-wrapper').height(0);
          }
        },
        afterTagRemoved: (event, ui) => {
          _this.adjustContentSlideWrapperHeight();
          _this.onSymbolsChange.apply(_this);
          return false;
        },
        allowDuplicates: false,
        readOnly: !_this.componentModel.isAdmin
      });
    });

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
    let parameters = this.componentModel.schema.parameters;
    for (let i=0; i<parameters.length; i++) {
      if (parameters[i].value.which == DataType.CURRENCY) {
        content.push(this.getCurrencyInput.apply(this, [i]));
      } else if (parameters[i].value.which == DataType.MONEY) {
        content.push(this.getMoneyInput.apply(this, [i]));
      } else if (parameters[i].value.which == DataType.LIST && parameters[i].value.value[0].which == DataType.SECURITY) {
        content.push(this.getSymbolsInput.apply(this, [i]));
      } else if (parameters[i].value.which == DataType.TIME_DURATION) {
        content.push(this.getPeriodInput.apply(this, [i]));
      } else if (parameters[i].value.which == DataType.INTEGER) {
        content.push(this.getIntegerInput.apply(this, [i]));
      } else if (parameters[i].value.which == DataType.BOOLEAN) {
        content.push(this.getBooleanInput.apply(this, [i]));
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
      </div>
    );
  }
}

export default View;
