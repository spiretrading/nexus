import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import Dropdown from 'components/reusables/common/dropdown';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import inputValidator from 'utils/input-validator';
import DataType from 'commons/data-types';
import deviceDetector from 'utils/device-detector';
import labelFormatter from 'utils/label-formatter';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isInitialized = false;
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
    let formattedNumber = numberFormatter.formatCurrencyWithComma(input);
    $input.val(formattedNumber);

    let value = formattedNumber.replace(',', '');
    let parameterName = $(event.currentTarget).attr('data-parameter-name');
    this.controller.onParameterUpdated.apply(this.controller, [
      parameterName,
      value
    ]);
  }

  /** @private */
  onSymbolsChange() {
    let securities = [];
    let $tagsUl = $('#' + this.componentModel.componentId + ' ul.symbols-input');
    $tagsUl.find('.tagit-label').each(function() {
      let label = $(this).text();
      let labelTokens = label.split('.');
      let symbol = labelTokens[0];
      let market = labelTokens[1];
      let country = definitionsService.getMarket(market).country_code;
      let security = {
        value: {
          country: country,
          market: market,
          symbol: symbol
        },
        which: 8
      };
      securities.push(security);
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
      options: currencyOptions
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
    let formattedNumber = numberFormatter.formatCurrencyWithComma(parameters[parameterIndex].value.value);
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
                 defaultValue={formattedNumber}/>
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
      if (security.symbol.length > 0 && security.market.length > 0) {
        let tagLabel = security.symbol + '.' + security.market;
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
    let ampm;
    if (parseInt(hour) >= 12) {
      ampm = 'PM';
      hour = (parseInt(hour) - 12).toString();
      if (hour.length == 1) {
        hour = '0' + hour;
      }
    } else {
      ampm = 'AM';
    }

    let input =
      <div className="entry-wrapper" key={parameterIndex}>
        <div className="name">{parameterName}</div>
        <div className="value">
          <div className="time-input-wrapper">
            <input className="hour numeric" type="text" size="2" maxLength="2" defaultValue={hour}/>:
            <input className="minute numeric" type="text" size="2" maxLength="2" defaultValue={minute}/>:
            <input className="second numeric" type="text" size="2" maxLength="2" defaultValue={second}/>
            <input className="ampm" type="text" size="3" maxLength="2" defaultValue={ampm}/>
          </div>
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
                 defaultValue={formattedNumber}/>
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
                 onChange={onBooleanInputChange}/>
        </div>
      </div>;

    return input;
  }

  initialize() {
    inputValidator.onlyNumbers($('#' + this.componentModel.componentId + ' .buying-power-input'));
    inputValidator.onlyNumbers($('#' + this.componentModel.componentId + ' .time-input-wrapper input.numeric'));
    inputValidator.onlyAmPm($('#' + this.componentModel.componentId + ' .time-input-wrapper input.ampm'));

    var _this = this;
    $('#' + this.componentModel.componentId + ' .symbols-input').tagit({
      autocomplete: {
        delay: 0,
        minLength: 1,
        source: (request, response) => {
          if (this.symbolsTimeout != null) {
            clearTimeout(this.symbolsTimeout);
          }

          this.symbolsTimeout = setTimeout(() => {
            this.symbolsTimeout = null;
            let input = $('#' + this.componentModel.componentId + ' .ui-autocomplete-input').val().trim();
            this.controller.searchSymbols(input)
              .then((results) => {
                let labels = [];
                for (let i=0; i<results.length; i++) {
                  let symbol = results[i].security.symbol + '.' + results[i].security.market;
                  labels.push({
                    label: symbol + ' (' + results[i].name + ')',
                    value: symbol
                  });
                }
                response(labels);
              });
          }, Config.INPUT_TIMEOUT_DURATION);
        }
      },
      beforeTagAdded: function(event, ui) {
        if (_this.isInitialized) {
          let tokens = ui.tagLabel.split('.');
          if (tokens.length !== 2){
            $(this).find('.ui-autocomplete-input').val('');
            return false;
          }
        }
      },
      afterTagAdded: (event, ui) => {
        if (this.isInitialized) {
          this.adjustContentSlideWrapperHeight();
          this.onSymbolsChange.apply(this);
        } else {
          $(event.currentTarget).parent().parent().find('.content-slide-wrapper').height(0);
        }
      },
      afterTagRemoved: (event, ui) => {
        this.adjustContentSlideWrapperHeight();
        this.onSymbolsChange.apply(this);
      },
      allowDuplicates: true
    });

    this.isInitialized = true;

    $('#' + this.componentModel.componentId + ' .boolean-input input').prop('checked', true);
  }

  render() {
    let name = this.componentModel.schema.name;

    let stateDropdownModel;
    if (this.componentModel.isGroup) {
      stateDropdownModel = {
        selectedValue: this.componentModel.state,
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

    let onStatusChange = this.controller.onStatusChange.bind(this.controller);

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
