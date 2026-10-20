import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DateExpression, dateExpressionToRule, DateOffset, DateRange,
  DateRangeExpressions,
  DateRangeOption, DateRangeRules, DateRangeValidation, DateRule, DateRuleType,
  DayOfMonth, isDateExpressionEqual, isDateRangeEqual, isDateRuleEqual,
  resolveDateExpression, resolveDateRule, SpecificDateRule, validateDateRange }
  from '../../models';
import { DateExpressionInput } from '../date_expression_input';
import { DateInput } from '../date_input';
import { Select } from '../select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange' | 'onInput'> {

  /** The optional label for the preset selector. */
  label?: string;

  /** The custom start and end labels. Defaults to Start and End. */
  customLabel?: {start?: string; end?: string};

  /** The custom date editor type. Defaults to FIXED. */
  customType?: DateRangeInput.CustomType;

  /** The committed range to display. */
  value: DateRange;

  /** The date rules. Defaults to a matching preset or fixed dates. */
  rules?: DateRangeRules;

  /** The date used to resolve relative rules. Defaults to today. */
  referenceDate?: Beam.Date;

  /** Reports custom rules independently of their resolved date values. */
  onRulesChange?: (rules: DateRangeRules) => void;

  /** The available presets. Custom is appended automatically. */
  options: readonly DateRangeOption[];

  /** The arrangement of the selector and dates. Defaults to VERTICAL. */
  orientation?: DateRangeInput.Orientation;

  /** The vertical layout's label placement. Defaults to ABOVE. */
  labelPosition?: DateRangeInput.LabelPosition;

  /** Whether both bounds must be specified. Defaults to true. */
  boundsRequired?: boolean;

  /** Whether the controls can only be viewed. */
  readOnly?: boolean;

  /** Whether the controls are disabled. */
  disabled?: boolean;

  /** Optional caller-controlled error presentation. */
  validation?: DateRangeValidation;

  /** Reports a valid range following a preset selection or date commit. */
  onChange?: (value: DateRange) => void;

  /** Reports validity and error presentation as the draft changes. */
  onValidationChange?: (validation: DateRangeValidation) => void;
}

interface State {
  value: DateRange;
  inputValue: DateRange;
  selection: string;
  startComplete: boolean;
  endComplete: boolean;
  showError: boolean;
  revision: number;
  rules: DateRangeRules;
  reference: Beam.Date;
  expressions: DateRangeExpressions;
}

/** Selects a preset date range or commits a validated custom range. */
export class DateRangeInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const reference = props.referenceDate ?? Beam.Date.today();
    const selection = findSelection(props.value, props.options, props.rules);
    const option = props.options.find(option => option.value === selection);
    const rules = props.rules ?? option?.rules ?? makeFixedRules(props.value);
    const expressions = props.value.expressions ??
      makeExpressions(rules, reference);
    const value = (() => {
      if(props.customType === DateRangeInput.CustomType.EXPRESSION &&
          props.value.expressions) {
        return resolveExpressions(expressions, reference);
      }
      return resolveRules(rules, reference);
    })();
    this.state = {value, inputValue: value, selection,
      startComplete: isComplete(rules.start, value.start),
      endComplete: isComplete(rules.end, value.end), showError: false,
      revision: 0, rules, reference, expressions};
    this.identifier = `date-range-${DateRangeInput.nextIdentifier++}`;
    this.submission = props.value;
    this.timer = null;
  }

  public componentDidMount(): void {
    this.publishValidation();
    this.scheduleMidnight();
    window.addEventListener('focus', this.onReferenceChange);
    document.addEventListener('visibilitychange', this.onReferenceChange);
  }

  public componentDidUpdate(previous: Properties): void {
    const rulesChanged = previous.rules !== this.props.rules &&
      this.props.rules &&
      (!isDateRuleEqual(this.props.rules.start, this.state.rules.start) ||
        !isDateRuleEqual(this.props.rules.end, this.state.rules.end));
    if(previous.value !== this.props.value) {
      this.submission = this.props.value;
    }
    let updated = false;
    if(this.props.value !== previous.value && this.props.value.expressions) {
      if(!isDateExpressionEqual(this.props.value.expressions.start,
          this.state.expressions.start) ||
          !isDateExpressionEqual(this.props.value.expressions.end,
            this.state.expressions.end) ||
          previous.referenceDate !== this.props.referenceDate ||
          !this.state.startComplete || !this.state.endComplete) {
        this.setExpressions(this.props.value.expressions, false, false);
      }
      updated = true;
    } else if(rulesChanged) {
      updated = true;
      this.setRules(this.props.rules, false);
      this.setState({selection: findSelection(this.props.value,
        this.props.options, this.props.rules)});
    } else if(previous.value !== this.props.value) {
      if(!isDateRangeEqual(this.props.value, this.state.value) ||
          !isDateExpressionEqual(this.props.value.expressions?.start,
            this.state.value.expressions?.start) ||
          !isDateExpressionEqual(this.props.value.expressions?.end,
            this.state.value.expressions?.end) ||
          !this.state.startComplete || !this.state.endComplete) {
        updated = true;
        const selection = findSelection(
          this.props.value, this.props.options, this.props.rules);
        const option = this.props.options.find(
          option => option.value === selection);
        this.setState(state => ({value: this.props.value,
          inputValue: this.props.value,
          expressions: makeExpressions(makeFixedRules(this.props.value),
            this.props.referenceDate ?? Beam.Date.today()),
          rules: option?.rules ?? makeFixedRules(this.props.value),
          reference: this.props.referenceDate ?? Beam.Date.today(),
          selection,
          startComplete: true, endComplete: true, showError: false,
          revision: state.revision + 1}), this.publishValidation);
      }
    } else if(previous.options !== this.props.options &&
        this.state.selection !== 'custom') {
      const option = this.props.options.find(
        option => option.value === this.state.selection);
      if(!option) {
        this.setState({selection: 'custom'});
      } else {
        const rules = option.rules ??
          makeFixedRules(new DateRange(option.startDate, option.endDate));
        const value = resolveRules(rules,
          this.props.referenceDate ?? Beam.Date.today());
        if(!isDateRangeEqual(value, this.state.value) ||
            !isDateRuleEqual(rules.start, this.state.rules.start) ||
            !isDateRuleEqual(rules.end, this.state.rules.end)) {
          updated = true;
          this.selectPreset(option);
        }
      }
    }
    if(previous.customType !== this.props.customType) {
      const selection = (() => {
        if(this.props.value.expressions) {
          return 'custom';
        } else if(previous.value !== this.props.value) {
          return findSelection(
            this.props.value, this.props.options, this.props.rules);
        }
        return this.state.selection;
      })();
      updated = true;
      const rules = (() => {
        if(previous.value !== this.props.value) {
          return this.props.rules ?? makeFixedRules(this.props.value);
        }
        return this.state.rules;
      })();
      if(this.props.customType === DateRangeInput.CustomType.EXPRESSION) {
        this.setExpressions(this.props.value.expressions ??
          makeExpressions(rules, this.props.referenceDate ?? Beam.Date.today()),
          false, false);
      } else {
        const value = (() => {
          if(previous.value !== this.props.value) {
            return this.props.value;
          }
          return this.state.value;
        })();
        this.setRules(makeFixedRules(value), false);
      }
      this.setState({selection});
    }
    if(previous.referenceDate !== this.props.referenceDate) {
      if(!updated) {
        this.onReferenceChange();
      }
      this.scheduleMidnight();
    }
    if(previous.boundsRequired !== this.props.boundsRequired) {
      this.publishValidation();
    }
  }

  public componentWillUnmount(): void {
    window.clearTimeout(this.timer);
    window.removeEventListener('focus', this.onReferenceChange);
    document.removeEventListener('visibilitychange', this.onReferenceChange);
  }

  public render(): JSX.Element {
    const {label, customLabel, customType, value, rules, referenceDate,
      onRulesChange, options,
      orientation, labelPosition, boundsRequired, readOnly, disabled,
      validation: supplied, onChange, onValidationChange,
      className, style, ...rest} = this.props;
    const expression = customType === DateRangeInput.CustomType.EXPRESSION;
    const horizontal = !expression &&
      orientation === DateRangeInput.Orientation.HORIZONTAL;
    const inline = !horizontal &&
      labelPosition === DateRangeInput.LabelPosition.INLINE;
    const labelInline = horizontal || inline;
    const custom = this.state.selection === 'custom';
    const validation = supplied ?? this.getValidation();
    const visibleError = validation.showError && !validation.valid;
    const {Target} = DateRangeValidation;
    const identifier = rest.id ?? this.identifier;
    const errorId = `${identifier}-error`;
    const startLabel = customLabel?.start ?? 'Start';
    const endLabel = customLabel?.end ?? 'End';
    const error = (target: DateRangeValidation.Target) => visibleError &&
      (validation.target === target ||
        validation.target === Target.START_AND_END);
    return <div {...rest} className={[css(STYLES.container,
        horizontal && STYLES.horizontal), className].join(' ')}
        style={style}>
      <SelectGroup id={`${identifier}-preset`} label={label}
        controls={`${identifier}-custom`} selection={this.state.selection}
        options={options} horizontal={horizontal} inline={inline}
        labelInline={labelInline}
        readOnly={readOnly} disabled={disabled} onChange={this.onSelect}/>
      <CustomDates id={`${identifier}-custom`} custom={custom}
          horizontal={horizontal} inline={inline}
          startId={`${identifier}-start`}
          endId={`${identifier}-end`} errorId={errorId} label={label}
          validation={validation} preview={this.state.value}
          startLabel={startLabel} endLabel={endLabel} expression={expression}
          start={(() => {
            if(expression) {
              return <DateExpressionInput key={`start-${this.state.revision}`}
                id={`${identifier}-start`} aria-label={startLabel}
                value={this.state.expressions.start}
                aria-invalid={error(Target.START)} aria-describedby={errorId}
                readOnly={readOnly} disabled={disabled || !custom}
                onInput={this.onStartExpressionInput}
                onChange={this.onStartExpressionChange}/>;
            }
            return <BoundDate key={`start-${this.state.revision}`}
              id={`${identifier}-start`} label={startLabel}
              value={this.state.inputValue.start} error={error(Target.START)}
              errorId={errorId} readOnly={readOnly}
              disabled={disabled || !custom}
              onInput={this.onStartInput} onCommit={this.onCommit}/>;
          })()}
          end={(() => {
            if(expression) {
              return <DateExpressionInput key={`end-${this.state.revision}`}
                id={`${identifier}-end`} aria-label={endLabel}
                value={this.state.expressions.end}
                aria-invalid={error(Target.END)} aria-describedby={errorId}
                readOnly={readOnly} disabled={disabled || !custom}
                onInput={this.onEndExpressionInput}
                onChange={this.onEndExpressionChange}/>;
            }
            return <BoundDate key={`end-${this.state.revision}`}
              id={`${identifier}-end`} label={endLabel}
              value={this.state.inputValue.end} error={error(Target.END)}
              errorId={errorId} readOnly={readOnly}
              disabled={disabled || !custom}
              onInput={this.onEndInput} onCommit={this.onCommit}/>;
          })()}/>
    </div>;
  }

  private getValidation(): DateRangeValidation {
    const {Error, Target} = DateRangeValidation;
    let validation =
      validateDateRange(this.state.value, this.props.boundsRequired !== false);
    const expression = this.props.customType ===
      DateRangeInput.CustomType.EXPRESSION;
    const startComplete = this.state.startComplete && (() => {
      if(expression) {
        return isExpressionComplete(this.state.expressions.start,
          this.state.value.start);
      }
      return isComplete(this.state.rules.start,
        resolveDateRule(this.state.rules.start, this.state.reference));
    })();
    const endComplete = this.state.endComplete && (() => {
      if(expression) {
        return isExpressionComplete(this.state.expressions.end,
          this.state.value.end);
      }
      return isComplete(this.state.rules.end,
        resolveDateRule(this.state.rules.end, this.state.reference));
    })();
    if(!startComplete || !endComplete) {
      let target = Target.START_AND_END;
      if(startComplete) {
        target = Target.END;
      } else if(endComplete) {
        target = Target.START;
      }
      validation = new DateRangeValidation(Error.FORMAT, target, false);
    }
    return new DateRangeValidation(validation.error, validation.target,
      this.state.showError);
  }

  private setRules(rules: DateRangeRules, publish: boolean): void {
    const reference = this.props.referenceDate ?? Beam.Date.today();
    const value = resolveRules(rules, reference);
    if(!publish) {
      this.submission = value;
    }
    this.setState(state => {
      const preserveStart = publish && !state.startComplete &&
        isDateRuleEqual(rules.start, state.rules.start);
      const preserveEnd = publish && !state.endComplete &&
        isDateRuleEqual(rules.end, state.rules.end);
      let startInput = value.start;
      let endInput = value.end;
      let revision = state.revision;
      if(preserveStart) {
        startInput = state.inputValue.start;
      }
      if(preserveEnd) {
        endInput = state.inputValue.end;
      }
      if(!publish) {
        ++revision;
      }
      return {rules, reference, value,
        expressions: makeExpressions(rules, reference),
        inputValue: new DateRange(startInput, endInput), revision,
        selection: 'custom',
        startComplete: !preserveStart && isComplete(rules.start, value.start),
        endComplete: !preserveEnd && isComplete(rules.end, value.end),
        showError: publish};
    }, () => {
      if(publish) {
        this.onCommit();
      } else {
        this.publishValidation();
      }
    });
  }

  private setExpressions(expressions: DateRangeExpressions, commit: boolean,
      input: boolean, completeness?: {start?: boolean; end?: boolean}): void {
    if((commit || input) && (this.props.readOnly || this.props.disabled)) {
      return;
    }
    const reference = this.props.referenceDate ?? Beam.Date.today();
    const value = resolveExpressions(expressions, reference);
    this.setState(state => ({expressions, reference, value, rules: value.rules,
      inputValue: value, selection: 'custom',
      revision: state.revision + Number(!commit && !input),
      startComplete: completeness?.start ?? (() => {
        if((commit || input) && isDateExpressionEqual(expressions.start,
            state.expressions.start)) {
          return state.startComplete;
        }
        return isExpressionComplete(expressions.start, value.start);
      })(),
      endComplete: completeness?.end ?? (() => {
        if((commit || input) && isDateExpressionEqual(expressions.end,
            state.expressions.end)) {
          return state.endComplete;
        }
        return isExpressionComplete(expressions.end, value.end);
      })(),
      showError: commit}), () => {
        if(commit) {
          this.onCommit();
        } else {
          this.publishValidation();
        }
      });
  }

  private selectPreset(option: DateRangeOption): void {
    const reference = this.props.referenceDate ?? Beam.Date.today();
    const rules = option.rules ??
      makeFixedRules(new DateRange(option.startDate, option.endDate));
    const value = resolveRules(rules, reference);
    this.setState(state => ({value, inputValue: value,
      selection: option.value, reference, rules,
      expressions: makeExpressions(rules, reference),
      startComplete: isComplete(rules.start, value.start),
      endComplete: isComplete(rules.end, value.end), showError: false,
      revision: state.revision + 1}), () => {
        this.publishValidation();
        this.props.onRulesChange?.(this.state.rules);
        if(this.getValidation().valid) {
          this.submission = value;
          this.props.onChange?.(value);
        }
      });
  }

  private scheduleMidnight(): void {
    window.clearTimeout(this.timer);
    if(!this.props.referenceDate) {
      const now = new Date();
      const next = new Date(now.getFullYear(), now.getMonth(),
        now.getDate() + 1);
      this.timer = window.setTimeout(this.onReferenceChange,
        next.getTime() - now.getTime());
    }
  }

  private publishValidation = () => {
    this.props.onValidationChange?.(this.getValidation());
  };

  private onSelect = (selection: string) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    if(selection === 'custom') {
      this.setState({selection, showError: false}, this.publishValidation);
      return;
    }
    const option = this.props.options.find(
      option => option.value === selection);
    if(!option) {
      return;
    }
    this.selectPreset(option);
  };

  private onStartInput = (start: Beam.Date, startComplete: boolean) => {
    this.setState(state => ({value: new DateRange(start, state.value.end),
      rules: new DateRangeRules(new SpecificDateRule(start),
        state.rules.end), startComplete, showError: false}),
      this.publishValidation);
  };

  private onEndInput = (end: Beam.Date, endComplete: boolean) => {
    this.setState(state => ({value: new DateRange(state.value.start, end),
      rules: new DateRangeRules(state.rules.start,
        new SpecificDateRule(end)), endComplete, showError: false}),
      this.publishValidation);
  };

  private onCommit = () => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    this.setState({showError: true}, () => {
      this.publishValidation();
      if(this.props.customType !== DateRangeInput.CustomType.EXPRESSION) {
        this.props.onRulesChange?.(this.state.rules);
      }
      if(this.getValidation().valid &&
          (!isDateRangeEqual(this.state.value, this.submission) ||
            !isDateExpressionEqual(this.state.value.expressions?.start,
              this.submission.expressions?.start) ||
            !isDateExpressionEqual(this.state.value.expressions?.end,
              this.submission.expressions?.end))) {
        this.submission = this.state.value;
        this.props.onChange?.(this.state.value);
      }
    });
  };

  private onStartExpressionInput = (start: DateExpression,
      complete?: boolean) => {
    this.setExpressions(new DateRangeExpressions(start,
      this.state.expressions.end), false, true, {start: complete});
  };

  private onEndExpressionInput = (end: DateExpression,
      complete?: boolean) => {
    this.setExpressions(new DateRangeExpressions(this.state.expressions.start,
      end), false, true, {end: complete});
  };

  private onStartExpressionChange = (start: DateExpression,
      complete?: boolean) => {
    this.setExpressions(new DateRangeExpressions(start,
      this.state.expressions.end), true, false, {start: complete});
  };

  private onEndExpressionChange = (end: DateExpression,
      complete?: boolean) => {
    this.setExpressions(new DateRangeExpressions(this.state.expressions.start,
      end), true, false, {end: complete});
  };

  private onReferenceChange = () => {
    const reference = this.props.referenceDate ?? Beam.Date.today();
    if(!reference.equals(this.state.reference)) {
      if(this.state.selection === 'custom') {
        if(this.props.customType === DateRangeInput.CustomType.EXPRESSION) {
          this.setExpressions(this.state.expressions, false, false);
        } else {
          this.setRules(this.state.rules, true);
        }
      } else {
        const option = this.props.options.find(
          option => option.value === this.state.selection);
        if(option?.rules) {
          this.selectPreset(option);
        } else {
          this.setState({reference});
        }
      }
    }
    this.scheduleMidnight();
  };

  private static nextIdentifier = 0;
  private identifier: string;
  private submission: DateRange;
  private timer: number;
}

export namespace DateRangeInput {

  /** The editors used for custom bounds. */
  export enum CustomType {

    /** Fixed calendar date inputs. */
    FIXED,

    /** Fixed or relative date expression inputs. */
    EXPRESSION
  }


  /** The arrangement of the selector and custom date inputs. */
  export enum Orientation {

    /** The date inputs appear below the selector. */
    VERTICAL,

    /** The date inputs appear beside the selector. */
    HORIZONTAL
  }

  /** The label placement in a vertical arrangement. */
  export enum LabelPosition {

    /** Labels appear above their inputs. */
    ABOVE,

    /** Labels appear to the left of their inputs. */
    INLINE
  }
}

interface CustomDatesProperties {
  id: string;
  custom: boolean;
  horizontal: boolean;
  inline: boolean;
  startId: string;
  endId: string;
  errorId: string;
  label: string;
  startLabel: string;
  endLabel: string;
  expression: boolean;
  validation: DateRangeValidation;
  start: React.ReactNode;
  end: React.ReactNode;
  preview: DateRange;
}

interface CustomDatesState {
  maximum: number;
}

/** Arranges custom bounds and applies the active layout's height constraint. */
class CustomDates extends
    React.Component<CustomDatesProperties, CustomDatesState> {
  constructor(props: CustomDatesProperties) {
    super(props);
    this.state = {maximum: 0};
    this.content = React.createRef<HTMLDivElement>();
    this.observer = null;
    this.frame = null;
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(this.updateHeight);
    this.observer.observe(this.content.current);
    this.scheduleHeight();
  }

  public componentDidUpdate(): void {
    this.scheduleHeight();
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    if(this.frame !== null) {
      window.cancelAnimationFrame(this.frame);
    }
  }

  public render(): JSX.Element {
    const props = this.props;
    const bounds: React.CSSProperties = {display: 'flex', minWidth: 0};
    const bound: React.CSSProperties = {display: 'grid', minWidth: 0};
    const labelStyle: React.CSSProperties = {lineHeight: '16px',
      alignSelf: 'start', boxSizing: 'border-box'};
    if(props.horizontal) {
      bounds.gap = '18px';
      bound.gridTemplateColumns = 'max-content 150px';
      bound.gap = '8px';
      labelStyle.alignSelf = 'center';
    } else if(props.inline) {
      bounds.flexDirection = 'column';
      bounds.gap = (() => {
        if(props.expression) {
          return '18px';
        }
        return '10px';
      })();
      bound.gridTemplateColumns = INLINE_COLUMNS;
      bound.gap = '8px';
      labelStyle.paddingTop = '9px';
      labelStyle.minHeight = '34px';
      if(props.label) {
        labelStyle.paddingInlineStart = '24px';
      }
    } else {
      bounds.flexDirection = 'column';
      bounds.gap = '20px';
      bound.gap = '12px';
      labelStyle.paddingInlineStart = '10px';
    }
    const style: React.CSSProperties = {maxHeight: this.state.maximum,
      minWidth: 0, overflow: 'clip',
      transition: 'max-height 200ms ease-in-out'};
    if(props.horizontal) {
      style.flex = '0 0 auto';
      if(props.custom) {
        style.marginLeft = '18px';
      } else {
        style.width = 0;
      }
    } else {
      style.width = '100%';
    }
    if(!props.custom) {
      style.visibility = 'hidden';
      style.transition += ', visibility 0s 200ms';
    }
    const contentStyle: React.CSSProperties = {};
    if(!props.horizontal) {
      if(props.inline) {
        contentStyle.paddingTop = '10px';
      } else {
        contentStyle.paddingTop = '18px';
      }
    }
    return <div id={props.id} style={style}
        aria-hidden={!props.custom}>
      <div ref={this.content} style={contentStyle}>
        <div style={bounds}>
          <div style={bound}>
            <label htmlFor={props.startId} style={labelStyle}>
              {props.startLabel}
            </label>
            {props.start}
          </div>
          <div style={bound}>
            <label htmlFor={props.endId} style={labelStyle}>
              {props.endLabel}
            </label>
            {props.end}
          </div>
        </div>
        <Error id={props.errorId} label={props.label}
          inline={props.inline} validation={props.validation}/>
        {props.expression && <ResolvedDateBlock inline={props.inline}
          value={props.preview}/>}
      </div>
    </div>;
  }

  private scheduleHeight(): void {
    if(this.frame !== null) {
      return;
    }
    this.frame = window.requestAnimationFrame(() => {
      this.frame = null;
      this.updateHeight();
    });
  }

  private updateHeight = () => {
    let maximum = 0;
    if(this.props.custom) {
      maximum = this.content.current.getBoundingClientRect().height;
    }
    if(this.state.maximum !== maximum) {
      this.setState({maximum});
    }
  };

  private content: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
  private frame: number;
}

function Error(props: {id: string; label: string; inline: boolean;
    validation: DateRangeValidation}): JSX.Element {
  const visible = props.validation.showError && !props.validation.valid;
  const style: React.CSSProperties = {paddingTop: '4px',
    color: '#E63F44', overflowWrap: 'anywhere'};
  if(props.inline) {
    style.paddingInlineStart = 'calc(58px + (100cqw - 246px) * (80 / 138))';
  }
  if(!visible) {
    style.maxHeight = 0;
    style.paddingTop = 0;
    style.overflow = 'hidden';
    style.visibility = 'hidden';
  }
  let message = '';
  const {Error} = DateRangeValidation;
  if(props.validation.error === Error.REQUIRED) {
    message = `${props.label || 'Date range'} cannot be empty`;
  } else if(props.validation.error === Error.FORMAT) {
    message = 'A valid start and end date is required';
  } else if(props.validation.error === Error.OUT_OF_RANGE) {
    message = 'End date must be greater than start date';
  }
  return <div id={props.id} style={style} aria-hidden={!visible}>
    {visible && <span role='alert'>{message}</span>}
  </div>;
}

function resolveExpressions(expressions: DateRangeExpressions,
    reference: Beam.Date): DateRange {
  return new DateRange(resolveDateExpression(expressions.start, reference),
    resolveDateExpression(expressions.end, reference),
    new DateRangeRules(dateExpressionToRule(expressions.start),
      dateExpressionToRule(expressions.end)), expressions);
}

function isExpressionComplete(expression: DateExpression,
    value: Beam.Date): boolean {
  return expression.mode === DateExpression.Mode.FIXED &&
    expression.date == null || value != null;
}

function makeExpressions(rules: DateRangeRules, reference: Beam.Date):
    DateRangeExpressions {
  const convert = (rule: DateRule): DateExpression => {
    if(rule.type === DateRuleType.DAY_OFFSET) {
      return {mode: DateExpression.Mode.DAY_OFFSET,
        date: {value: rule.offset, unit: DateOffset.Unit.DAY}};
    } else if(rule.type === DateRuleType.WEEKDAY) {
      return {mode: DateExpression.Mode.WEEKDAY,
        date: {day: rule.day,
          offset: {value: rule.offset, unit: DateOffset.Unit.WEEK}}};
    } else if(rule.type === DateRuleType.DAY_OF_MONTH) {
      return {mode: DateExpression.Mode.DAY_OF_MONTH,
        date: {day: new DayOfMonth(DayOfMonth.Position.DAY, rule.day),
          offset: {value: rule.offset, unit: DateOffset.Unit.MONTH}}};
    } else if(rule.type === DateRuleType.ORDINAL_DAY_OF_MONTH) {
      return {mode: DateExpression.Mode.DAY_OF_MONTH,
        date: {day: new DayOfMonth(
          rule.position as unknown as DayOfMonth.Position,
          rule.day as unknown as DayOfMonth.DayType),
          offset: {value: rule.offset, unit: DateOffset.Unit.MONTH}}};
    } else if(rule.type === DateRuleType.MONTH_BOUNDARY &&
        rule.dayOffset === 0) {
      const position = (() => {
        if(rule.boundary === 'FIRST') {
          return DayOfMonth.Position.FIRST;
        }
        return DayOfMonth.Position.LAST;
      })();
      return {mode: DateExpression.Mode.DAY_OF_MONTH,
        date: {day: new DayOfMonth(position, DayOfMonth.DayType.DAY),
          offset: {value: rule.offset, unit: DateOffset.Unit.MONTH}}};
    }
    return {mode: DateExpression.Mode.FIXED,
      date: resolveDateRule(rule, reference)};
  };
  return new DateRangeExpressions(convert(rules.start), convert(rules.end));
}

function makeFixedRules(value: DateRange): DateRangeRules {
  return new DateRangeRules(new SpecificDateRule(value.start),
    new SpecificDateRule(value.end));
}

function resolveRules(rules: DateRangeRules, reference: Beam.Date): DateRange {
  return new DateRange(resolveDateRule(rules.start, reference),
    resolveDateRule(rules.end, reference), rules);
}

function isComplete(rule: DateRule, date: Beam.Date): boolean {
  return rule.type === DateRuleType.SPECIFIC_DATE && rule.date == null ||
    date != null;
}

function formatPreview(date: Beam.Date): string {
  if(!date) {
    return 'Unbounded';
  }
  const value = new Date(0);
  value.setUTCFullYear(date.year, date.month - 1, date.day);
  return value.toLocaleDateString('en-US',
    {year: 'numeric', month: 'short', day: 'numeric', timeZone: 'UTC'});
}

interface SelectGroupProperties {
  id: string;
  label: string;
  controls: string;
  selection: string;
  options: readonly DateRangeOption[];
  horizontal: boolean;
  inline: boolean;
  labelInline: boolean;
  readOnly: boolean;
  disabled: boolean;
  onChange: (selection: string) => void;
}

function SelectGroup(props: SelectGroupProperties): JSX.Element {
  const style: React.CSSProperties = {minWidth: 0};
  const selectStyle: React.CSSProperties = {width: '100%', minWidth: 0,
    boxSizing: 'border-box'};
  if(props.horizontal) {
    style.flex = '0 0 auto';
  }
  if(props.label) {
    if(props.horizontal) {
      style.display = 'flex';
      style.alignItems = 'center';
      style.gap = '8px';
      selectStyle.width = '150px';
    } else if(props.inline) {
      style.display = 'grid';
      style.alignItems = 'center';
      style.gridTemplateColumns = INLINE_COLUMNS;
      style.gap = '8px';
    } else {
      style.display = 'flex';
      style.flexDirection = 'column';
      style.gap = '12px';
    }
  }
  return <div style={style}>
    {props.label && <label htmlFor={props.id}
        style={{lineHeight: '16px', paddingInlineStart: (() => {
          if(props.labelInline) {
            return 0;
          }
          return '10px';
        })()}}>
      {props.label}
    </label>}
    <Select id={props.id} value={props.selection} style={selectStyle}
        aria-label={props.label || 'Date range'}
        aria-controls={props.controls}
        readOnly={props.readOnly} disabled={props.disabled}
        onChange={props.onChange}>
      {props.options.map(option => <option key={option.value}
        value={option.value}>{option.label}</option>)}
      <option value='custom'>Custom</option>
    </Select>
  </div>;
}


function findSelection(value: DateRange, options: readonly DateRangeOption[],
    rules?: DateRangeRules): string {
  if(value.expressions) {
    return 'custom';
  }
  return options.find(option => {
    if(rules) {
      const preset = option.rules ??
        makeFixedRules(new DateRange(option.startDate, option.endDate));
      return isDateRuleEqual(rules.start, preset.start) &&
        isDateRuleEqual(rules.end, preset.end);
    }
    return isDateRangeEqual(value,
      new DateRange(option.startDate, option.endDate));
  })?.value ?? 'custom';
}

const INLINE_COLUMNS =
  'calc(50px + (100cqw - 246px) * (80 / 138)) minmax(0, 1fr)';
const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%',
    fontSize: '14px', fontWeight: 400, color: '#333333'},
  horizontal: {display: 'flex', alignItems: 'flex-start'}
});

function ResolvedDateBlock(props: {inline: boolean; value: DateRange}):
    JSX.Element {
  const style: React.CSSProperties = {paddingTop: '12px', fontSize: '0.75rem',
    lineHeight: '14px', color: '#5D5E6D'};
  if(props.inline) {
    style.paddingInlineStart = 'calc(58px + (100cqw - 246px) * (80 / 138))';
  }
  return <div aria-live='polite' aria-label='Resolved date range' style={style}>
    {formatPreview(props.value.start)} {'\u2013'}
    {' '}{formatPreview(props.value.end)}
  </div>;
}

interface BoundDateProperties {
  id: string;
  label: string;
  value: Beam.Date;
  error: boolean;
  errorId: string;
  readOnly: boolean;
  disabled: boolean;
  onInput: (value: Beam.Date, complete: boolean) => void;
  onCommit: () => void;
}

/** Adapts segmented date editing to range input and commit events. */
class BoundDate extends React.Component<BoundDateProperties> {
  constructor(props: BoundDateProperties) {
    super(props);
    this.element = React.createRef<HTMLDivElement>();
    this.hasNormalizedValue = false;
  }

  public render(): JSX.Element {
    return <div ref={this.element} role='group' aria-label={this.props.label}
        aria-invalid={this.props.error} aria-describedby={this.props.errorId}
        onInput={this.onInput} onBlur={this.onBlur}
        onKeyDown={this.onKeyDown}>
      <DateInput id={this.props.id} value={this.props.value}
        error={this.props.error} readOnly={this.props.readOnly}
        disabled={this.props.disabled} style={{width: '100%'}}
        aria-invalid={this.props.error}
        aria-describedby={this.props.errorId} onChange={this.onChange}/>
    </div>;
  }

  private onInput = () => {
    this.hasNormalizedValue = false;
    const parts = Array.from(this.element.current.querySelectorAll('input'),
      input => input.value);
    if(parts.every(part => part === '')) {
      this.props.onInput(null, true);
      return;
    }
    if(parts.some(part => !/^\d+$/.test(part))) {
      this.props.onInput(null, false);
      return;
    }
    const [year, month, day] = parts.map(Number);
    const date = new Date(0);
    date.setUTCFullYear(year, month - 1, day);
    if(year > 9999 || date.getUTCFullYear() !== year ||
        date.getUTCMonth() !== month - 1 || date.getUTCDate() !== day) {
      this.props.onInput(null, false);
      return;
    }
    this.props.onInput(new Beam.Date(year, month, day), true);
  };

  private onChange = (value: Beam.Date) => {
    this.hasNormalizedValue = true;
    this.props.onInput(value ?? null, true);
  };

  private onBlur = (event: React.FocusEvent) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    const commit = !event.currentTarget.contains(event.relatedTarget as Node);
    this.forceUpdate(() => {
      if(this.props.readOnly || this.props.disabled) {
        return;
      }
      if(!this.hasNormalizedValue) {
        this.onInput();
      }
      if(commit) {
        this.props.onCommit?.();
      }
    });
  };

  private onKeyDown = (event: React.KeyboardEvent) => {
    if(event.key === 'Enter' && !event.nativeEvent.isComposing &&
        !this.props.readOnly && !this.props.disabled) {
      event.preventDefault();
      const input = event.target as HTMLInputElement;
      input.blur();
      input.focus();
    }
  };

  private element: React.RefObject<HTMLDivElement>;
  private hasNormalizedValue: boolean;
}
