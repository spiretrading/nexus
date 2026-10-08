import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DateRange, DateRangeOption, DateRangeRules, DateRangeValidation,
  DateRule, isDateRangeEqual, isDateRuleEqual, resolveDateRule,
  validateDateRange } from '../../models';
import { DateInput } from '../date_input';
import { IntegerInput } from '../integer_input';
import { Select } from '../select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange' | 'onInput'> {

  /** The optional label for the preset selector. */
  label?: string;

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
}

/** Selects a preset date range or commits a validated custom range. */
export class DateRangeInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const reference = props.referenceDate ?? Beam.Date.today();
    const selection = findSelection(props.value, props.options);
    const option = props.options.find(option => option.value === selection);
    const rules = props.rules ?? option?.rules ?? makeFixedRules(props.value);
    const value = resolveRules(rules, reference);
    this.state = {value, inputValue: value,
      selection: (() => {
        if(props.rules) {
          return 'custom';
        }
        return selection;
      })(), startComplete: isComplete(rules.start, value.start),
      endComplete: isComplete(rules.end, value.end), showError: false,
      revision: 0, rules, reference};
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
    if(rulesChanged) {
      updated = true;
      this.setRules(this.props.rules, false);
    } else if(previous.value !== this.props.value) {
      if(!isDateRangeEqual(this.props.value, this.state.value) ||
          !this.state.startComplete || !this.state.endComplete) {
        updated = true;
        const selection = findSelection(this.props.value, this.props.options);
        const option = this.props.options.find(
          option => option.value === selection);
        this.setState(state => ({value: this.props.value,
          inputValue: this.props.value,
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
    const {label, value, rules, referenceDate, onRulesChange, options,
      orientation, labelPosition, boundsRequired, readOnly, disabled,
      validation: supplied, onChange, onValidationChange,
      className, style, ...rest} = this.props;
    const horizontal = orientation === DateRangeInput.Orientation.HORIZONTAL;
    const inline = !horizontal &&
      labelPosition === DateRangeInput.LabelPosition.INLINE;
    const labelInline = labelPosition === DateRangeInput.LabelPosition.INLINE;
    const custom = this.state.selection === 'custom';
    const validation = supplied ?? this.getValidation();
    const visibleError = validation.showError && !validation.valid;
    const {Target} = DateRangeValidation;
    const identifier = rest.id ?? this.identifier;
    const errorId = `${identifier}-error`;
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
          horizontal={horizontal} inline={inline} labelInline={labelInline}
          startId={`${identifier}-start`}
          endId={`${identifier}-end`} errorId={errorId} label={label}
          validation={validation} preview={this.state.value}
          start={<DateRuleInput key={`start-${this.state.revision}`}
            id={`${identifier}-start`} label='From'
            value={this.state.rules.start}
            dateValue={this.state.inputValue.start}
            resolved={this.state.value.start} reference={this.state.reference}
            error={error(Target.START)} errorId={errorId} readOnly={readOnly}
            disabled={disabled || !custom} onChange={this.onStartRule}
            onInput={this.onStartInput} onCommit={this.onCommit}/>}
          end={<DateRuleInput key={`end-${this.state.revision}`}
            id={`${identifier}-end`} label='To'
            value={this.state.rules.end} dateValue={this.state.inputValue.end}
            resolved={this.state.value.end} reference={this.state.reference}
            error={error(Target.END)} errorId={errorId} readOnly={readOnly}
            disabled={disabled || !custom} onChange={this.onEndRule}
            onInput={this.onEndInput} onCommit={this.onCommit}/>}/>
    </div>;
  }

  private getValidation(): DateRangeValidation {
    const {Error, Target} = DateRangeValidation;
    let validation = validateDateRange(this.state.value,
      this.props.boundsRequired !== false);
    if(!this.state.startComplete || !this.state.endComplete) {
      let target = Target.START_AND_END;
      if(this.state.startComplete) {
        target = Target.END;
      } else if(this.state.endComplete) {
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

  private selectPreset(option: DateRangeOption): void {
    const reference = this.props.referenceDate ?? Beam.Date.today();
    const rules = option.rules ??
      makeFixedRules(new DateRange(option.startDate, option.endDate));
    const value = resolveRules(rules, reference);
    this.setState(state => ({value, inputValue: value,
      selection: option.value, reference, rules,
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
      rules: new DateRangeRules({...state.rules.start, date: start},
        state.rules.end), startComplete, showError: false}),
      this.publishValidation);
  };

  private onEndInput = (end: Beam.Date, endComplete: boolean) => {
    this.setState(state => ({value: new DateRange(state.value.start, end),
      rules: new DateRangeRules(state.rules.start,
        {...state.rules.end, date: end}), endComplete, showError: false}),
      this.publishValidation);
  };

  private onCommit = () => {
    this.setState({showError: true}, () => {
      this.publishValidation();
      this.props.onRulesChange?.(this.state.rules);
      if(this.getValidation().valid &&
          !isDateRangeEqual(this.state.value, this.submission)) {
        this.submission = this.state.value;
        this.props.onChange?.(this.state.value);
      }
    });
  };

  private onStartRule = (start: DateRule) => {
    this.setRules(new DateRangeRules(start, this.state.rules.end), true);
  };

  private onEndRule = (end: DateRule) => {
    this.setRules(new DateRangeRules(this.state.rules.start, end), true);
  };

  private onReferenceChange = () => {
    const reference = this.props.referenceDate ?? Beam.Date.today();
    if(!reference.equals(this.state.reference)) {
      if(this.state.selection === 'custom') {
        this.setRules(this.state.rules, true);
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
  labelInline: boolean;
  startId: string;
  endId: string;
  errorId: string;
  label: string;
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
    this.element = React.createRef<HTMLDivElement>();
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
    const bounds: React.CSSProperties = {display: 'flex', gap: '18px'};
    const bound: React.CSSProperties = {display: 'grid', minWidth: 0,
      gap: '8px', alignContent: 'start'};
    if(props.horizontal) {
      bounds.flexWrap = 'wrap';
      bound.flex = '1 1 300px';
    } else {
      bounds.flexDirection = 'column';
    }
    const boundClass = css(props.inline && STYLES.inlineBound,
      props.horizontal && props.labelInline && STYLES.horizontalInlineBound);
    const style: React.CSSProperties = {maxHeight: this.state.maximum,
      minWidth: 0, flex: '1 1 480px'};
    if(!props.horizontal) {
      style.width = '100%';
    }
    if(!props.custom) {
      style.display = 'none';
      style.visibility = 'hidden';
      style.overflow = 'clip';
    } else {
      style.transition = 'max-height 200ms ease-in-out';
      style.overflow = 'clip';
    }
    const contentStyle: React.CSSProperties = {};
    if(!props.horizontal) {
      contentStyle.paddingTop = '18px';
    }
    const labelStyle: React.CSSProperties = {display: 'flex',
      alignItems: 'center', alignSelf: 'start', minHeight: '20px'};
    if(props.labelInline) {
      labelStyle.minHeight = '34px';
    }
    return <div id={props.id} ref={this.element} style={style}
        aria-hidden={!props.custom}>
      <div ref={this.content} style={contentStyle}>
        <div style={bounds}>
          <div className={boundClass} style={bound}>
            <label htmlFor={props.startId} style={labelStyle}>From</label>
            {props.start}
          </div>
          <div className={boundClass} style={bound}>
            <label htmlFor={props.endId} style={labelStyle}>To</label>
            {props.end}
          </div>
        </div>
        <Error id={props.errorId} label={props.label}
          inline={false} validation={props.validation}/>
        {props.validation.valid && <div aria-live='polite'
            aria-label='Resolved date range'
            style={{paddingTop: '12px', fontSize: '12px', color: '#7D7E90'}}>
          {formatPreview(props.preview.start)} {'\u2013'}
          {' '}{formatPreview(props.preview.end)}
        </div>}
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

  private element: React.RefObject<HTMLDivElement>;
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
        disabled={this.props.disabled} aria-invalid={this.props.error}
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
        this.props.onCommit();
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

interface DateRuleInputProperties extends Omit<BoundDateProperties, 'value'> {
  value: DateRule;
  dateValue: Beam.Date;
  resolved: Beam.Date;
  reference: Beam.Date;
  onChange: (rule: DateRule) => void;
}

class DateRuleInput extends React.Component<DateRuleInputProperties> {
  public render(): JSX.Element {
    const props = this.props;
    const rule = props.value;
    const specific = rule.type === DateRule.Type.SPECIFIC_DATE;
    const style: React.CSSProperties = {width: '100%', minWidth: 0,
      boxSizing: 'border-box'};
    const unit = (() => {
      if(rule.type === DateRule.Type.DAY_OFFSET) {
        return 'Days';
      } else if(rule.type === DateRule.Type.WEEKDAY) {
        return 'Weeks';
      }
      return 'Months';
    })();
    return <div role='group' aria-label={`${props.label} date rule`}
        style={{display: 'grid', gap: '8px', minWidth: 0}}>
      <Select id={props.id} aria-label={`${props.label} rule`}
          value={rule.type} style={style} readOnly={props.readOnly}
          disabled={props.disabled} onChange={this.onTypeChange}>
        <option value={DateRule.Type.SPECIFIC_DATE}>Specific date</option>
        <option value={DateRule.Type.DAY_OFFSET}>Day offset</option>
        <option value={DateRule.Type.WEEKDAY}>Weekday</option>
        <option value={DateRule.Type.DAY_OF_MONTH}>Day of month</option>
        <option value={DateRule.Type.MONTH_BOUNDARY}>Month boundary</option>
      </Select>
      {specific && <BoundDate id={`${props.id}-date`}
        label={`${props.label} date`} value={props.dateValue}
        error={props.error} errorId={props.errorId} readOnly={props.readOnly}
        disabled={props.disabled} onInput={props.onInput}
        onCommit={props.onCommit}/>}
      {rule.type === DateRule.Type.WEEKDAY &&
        <Select aria-label={`${props.label} weekday`} value={rule.weekday}
            style={style} readOnly={props.readOnly} disabled={props.disabled}
            onChange={this.onWeekdayChange}>
          {['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday',
            'Sunday'].map((day, index) =>
              <option value={index} key={day}>{day}</option>)}
        </Select>}
      {rule.type === DateRule.Type.DAY_OF_MONTH &&
        <label style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
          Day
          <IntegerInput aria-label={`${props.label} day of month`}
            value={rule.day} min={1} max={31} step={1} inputMode='numeric'
            readOnly={props.readOnly} disabled={props.disabled}
            aria-invalid={props.error} aria-describedby={props.errorId}
            style={{width: '64px', minWidth: 0}} onChange={this.onDayChange}/>
        </label>}
      {rule.type === DateRule.Type.MONTH_BOUNDARY &&
        <div style={{display: 'grid', gap: '6px',
            gridTemplateColumns: '40px minmax(0, 1.2fr) minmax(0, 1fr)'}}>
          <IntegerInput aria-label={`${props.label} boundary day offset`}
            value={rule.boundaryOffset} min={0} step={1} inputMode='numeric'
            readOnly={props.readOnly} disabled={props.disabled}
            aria-invalid={props.error} aria-describedby={props.errorId}
            style={{width: '100%', minWidth: 0}}
            onChange={this.onBoundaryOffsetChange}/>
          <Select aria-label={`${props.label} boundary direction`}
              value={rule.boundaryDirection}
              style={{...style, paddingLeft: '6px', paddingRight: '20px',
                backgroundPosition: 'right 7px top 50%'}}
              readOnly={props.readOnly} disabled={props.disabled}
              onChange={this.onBoundaryDirectionChange}>
            <option value={DateRule.Direction.BEFORE}>Days before</option>
            <option value={DateRule.Direction.AFTER}>Days after</option>
          </Select>
          <Select aria-label={`${props.label} month boundary`}
              value={rule.boundary}
              style={{...style, paddingLeft: '6px', paddingRight: '20px',
                backgroundPosition: 'right 7px top 50%'}}
              readOnly={props.readOnly} disabled={props.disabled}
              onChange={this.onBoundaryChange}>
            <option value={DateRule.Boundary.FIRST}>First day</option>
            <option value={DateRule.Boundary.LAST}>Last day</option>
          </Select>
        </div>}
      {!specific && <div
          style={{display: 'flex', flexWrap: 'wrap', gap: '8px'}}>
        <IntegerInput aria-label={`${props.label} offset`} value={rule.count}
          min={0} step={1} inputMode='numeric' readOnly={props.readOnly}
          disabled={props.disabled} aria-invalid={props.error}
          aria-describedby={props.errorId}
          style={{width: '64px', flex: '0 0 64px', minWidth: 0}}
          onChange={this.onCountChange}/>
        <Select aria-label={`${props.label} direction`} value={rule.direction}
            style={{...style, flex: '1 1 116px', width: 'auto'}}
            readOnly={props.readOnly} disabled={props.disabled}
            onChange={this.onDirectionChange}>
          <option value={DateRule.Direction.BEFORE}>{unit} ago</option>
          <option value={DateRule.Direction.AFTER}>{unit} ahead</option>
        </Select>
      </div>}
    </div>;
  }

  private change(value: DateRule): void {
    if(!this.props.readOnly && !this.props.disabled) {
      this.props.onChange(value);
    }
  }

  private onTypeChange = (type: string) => {
    this.change(new DateRule(Number(type),
      this.props.resolved ?? this.props.reference));
  };

  private onWeekdayChange = (weekday: string) => {
    this.change({...this.props.value, weekday: Number(weekday)});
  };

  private onDayChange = (day: number) => {
    this.change({...this.props.value, day: day ?? null});
  };

  private onBoundaryChange = (boundary: string) => {
    this.change({...this.props.value, boundary: Number(boundary)});
  };

  private onBoundaryOffsetChange = (boundaryOffset: number) => {
    this.change({...this.props.value, boundaryOffset: boundaryOffset ?? null});
  };

  private onBoundaryDirectionChange = (direction: string) => {
    this.change({...this.props.value, boundaryDirection: Number(direction)});
  };

  private onCountChange = (count: number) => {
    this.change({...this.props.value, count: count ?? null});
  };

  private onDirectionChange = (direction: string) => {
    this.change({...this.props.value, direction: Number(direction)});
  };
}

function makeFixedRules(value: DateRange): DateRangeRules {
  return new DateRangeRules(new DateRule(DateRule.Type.SPECIFIC_DATE,
    value.start), new DateRule(DateRule.Type.SPECIFIC_DATE, value.end));
}

function resolveRules(rules: DateRangeRules, reference: Beam.Date): DateRange {
  return new DateRange(resolveDateRule(rules.start, reference),
    resolveDateRule(rules.end, reference));
}

function isComplete(rule: DateRule, date: Beam.Date): boolean {
  return rule.type === DateRule.Type.SPECIFIC_DATE || date != null;
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

class SelectGroup extends React.Component<SelectGroupProperties> {
  public render(): JSX.Element {
    const props = this.props;
    const style: React.CSSProperties = {flex: '1 1 0', minWidth: 0};
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
      {props.label && <label htmlFor={props.id}>
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
}

function findSelection(value: DateRange, options: readonly DateRangeOption[]):
    string {
  return options.find(option => isDateRangeEqual(value,
    new DateRange(option.startDate, option.endDate)))?.value ?? 'custom';
}

const INLINE_COLUMNS =
  'clamp(50px, calc(50px + (100cqw - 246px) * (80 / 138)), 130px) ' +
  'minmax(0, 1fr)';
const STYLES = StyleSheet.create({
  inlineBound: {
    '@container (min-width: 384px)': {gridTemplateColumns: INLINE_COLUMNS}
  },
  horizontalInlineBound: {
    '@container (min-width: 384px)': {
      gridTemplateColumns: 'max-content minmax(0, 1fr)'}
  },
  container: {containerType: 'inline-size', width: '100%',
    font: '400 14px Roboto, system-ui, sans-serif', color: '#333333'},
  horizontal: {display: 'flex', flexWrap: 'wrap',
    alignItems: 'flex-start', gap: '18px'}
});
