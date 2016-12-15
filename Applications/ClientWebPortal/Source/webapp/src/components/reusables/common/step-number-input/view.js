import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onValueChange(event) {
    let newValue = $('#' + this.componentModel.componentId + ' select').val();
    this.controller.onSelectionChange.apply(this.controller, [newValue]);
  }

  /** @private */
  onKeyDown(event) {
    if (event.keyCode === 38 || event.keyCode === 40) {
      let $input = $(event.currentTarget);
      let currentValue = Number($input.val());
      if (!$input.hasClass('invalid')) {
        if (event.keyCode === 38) {
          currentValue += this.componentModel.stepSize;
          if (this.componentModel.doRoll) {
            if (currentValue > this.componentModel.max) {
              currentValue = this.componentModel.min;
            }
          } else {
            if (currentValue > this.componentModel.max) {
              currentValue = this.componentModel.max;
            }
          }
        } else if (event.keyCode === 40) {
          currentValue -= this.componentModel.stepSize;
          if (this.componentModel.doRoll) {
            if (currentValue < this.componentModel.min) {
              currentValue = this.componentModel.max;
            }
          } else {
            if (currentValue < this.componentModel.min) {
              currentValue = this.componentModel.min;
            }
          }
        }

        $input.val(currentValue);
      }
    }
  }

  /** @private */
  onKeyUp(event) {
    clearInterval(this.incrementInterval);
    clearInterval(this.decrementInterval);

    let $input = $(event.currentTarget);
    let validationFailMessage = this.validateInput.apply(this, [$input.val()]);

    if (validationFailMessage == null) {
      $input.removeClass('invalid').addClass('valid');
      this.controller.onValueChange.apply(this.controller, [Number($input.val())]);
    } else {
      $input.removeClass('valid').addClass('invalid');
      this.controller.onValidationFail.apply(this.controller, [validationFailMessage]);
    }
  }

  /** @private */
  validateInput(input) {
    let inputNumber = Number(input);
    if (isNaN(inputNumber)) {
      return 'Not a number.';
    } else if (!this.isValidDecimals.apply(this, [input])) {
      if (this.componentModel.allowedDecimals > 0) {
        return 'Exceeded ' + this.componentModel.allowedDecimals + ' decimal places.';
      } else {
        return 'Decimal places are not allowed.';
      }
    } else if (this.componentModel.isRequired && (input == null || input.length == 0)) {
      return 'This field is required.';
    }

    return null;
  }

  /** @private */
  isValidDecimals(input) {
    let periodIndex = input.indexOf('.');
    if (periodIndex == -1) {
      return true;
    } else if (this.componentModel.allowedDecimals == 0) {
      return false;
    } else {
      let decimalPlaces = input.substring(periodIndex + 1);
      return decimalPlaces.length <= this.componentModel.allowedDecimals;
    }
  }

  /** @private */
  isUpScroll(event) {
    // chrome, safari
    if (event.originalEvent.wheelDelta != null) {
      return event.originalEvent.wheelDelta > 0;
    }

    // firefox
    if (event.originalEvent.detail != null) {
      return event.originalEvent.detail < 0;
    }

    return false;
  }

  /** @private */
  onArrowUpClick() {
    if (!this.componentModel.isDisabled) {
      let $input = $('#' + this.componentModel.componentId).find('input');
      let currentValue = Number($input.val());
      if (!$input.hasClass('invalid')) {
        currentValue += this.componentModel.stepSize;
        if (this.componentModel.doRoll) {
          if (currentValue > this.componentModel.max) {
            currentValue = this.componentModel.min;
          }
        } else {
          if (currentValue > this.componentModel.max) {
            currentValue = this.componentModel.max;
          }
        }
        $input.val(currentValue).focus();
        this.controller.onValueChange.apply(this.controller, [currentValue]);
      }
    }
  }

  /** @private */
  onArrowDownClick() {
    if (!this.componentModel.isDisabled) {
      let $input = $('#' + this.componentModel.componentId).find('input');
      let currentValue = Number($input.val());
      if (!$input.hasClass('invalid')) {
        currentValue -= this.componentModel.stepSize;
        if (this.componentModel.doRoll) {
          if (currentValue < this.componentModel.min) {
            currentValue = this.componentModel.max;
          }
        } else {
          if (currentValue < this.componentModel.min) {
            currentValue = this.componentModel.min;
          }
        }
        $input.val(currentValue).focus();
        this.controller.onValueChange.apply(this.controller, [currentValue]);
      }
    }
  }

  initialize() {
    $('#' + this.componentModel.componentId + ' input').focusin((event) => {
      $('#' + this.componentModel.componentId).bind('DOMMouseScroll mousewheel', (event) => {
        let $input = $('#' + this.componentModel.componentId).find('input');
        let currentValue = Number($input.val());
        if (!$input.hasClass('invalid')) {
          if(this.isUpScroll.apply(this, [event])) {
            currentValue += this.componentModel.stepSize;
            if (this.componentModel.doRoll) {
              if (currentValue > this.componentModel.max) {
                currentValue = this.componentModel.min;
              }
            } else {
              if (currentValue > this.componentModel.max) {
                currentValue = this.componentModel.max;
              }
            }
          } else {
            currentValue -= this.componentModel.stepSize;
            if (this.componentModel.doRoll) {
              if (currentValue < this.componentModel.min) {
                currentValue = this.componentModel.max;
              }
            } else {
              if (currentValue < this.componentModel.min) {
                currentValue = this.componentModel.min;
              }
            }
          }

          $input.val(currentValue);
          this.controller.onValueChange.apply(this.controller, [currentValue]);
        }
      });
    });

    $('#' + this.componentModel.componentId + ' input').focusout((e) => {
      $('#' + this.componentModel.componentId).unbind('DOMMouseScroll mousewheel');
    });
  }

  render() {
    let className = 'step-number-input-container';

    if (this.componentModel.className != null) {
      className = className + ' ' + this.componentModel.className;
    }

    if (this.componentModel.isDisabled) {
      className += ' disabled';
    }

    return (
        <div id={this.componentModel.componentId} className={className}>
          <input type="text"
                 placeholder={this.componentModel.placeholder}
                 onKeyDown={this.onKeyDown.bind(this)}
                 onKeyUp={this.onKeyUp.bind(this)}
                 maxLength={this.componentModel.maxLength}
                 defaultValue={this.componentModel.defaultValue}
                 disabled={this.componentModel.isDisabled}
          />
          <span className="icon-arrow-up" onClick={this.onArrowUpClick.bind(this)}/>
          <span className="icon-arrow-down" onClick={this.onArrowDownClick.bind(this)}/>
        </div>
    );
  }
}

export default View;
