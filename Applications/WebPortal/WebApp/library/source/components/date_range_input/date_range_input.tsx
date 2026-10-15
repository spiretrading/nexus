import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DateRange, DateRangeOption, DateRangeValidation, isDateRangeEqual,
  validateDateRange } from '../../models';
import { DateInput } from '../date_input';
import { Select } from '../select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange' | 'onInput'> {

  /** The optional label for the preset selector. */
  label?: string;

  /** The committed range to display. */
  value: DateRange;

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
  customHeight: number;
}

/** Selects a preset date range or commits a validated custom range. */
export class DateRangeInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {value: props.value, inputValue: props.value,
      selection: findSelection(props.value, props.options),
      startComplete: true, endComplete: true, showError: false,
      revision: 0, customHeight: 0};
    this.identifier = `date-range-${DateRangeInput.nextIdentifier++}`;
    this.submission = props.value;
  }

  public componentDidMount(): void {
    this.publishValidation();
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value) {
      this.submission = this.props.value;
      if(!isDateRangeEqual(this.props.value, this.state.value) ||
          !this.state.startComplete || !this.state.endComplete) {
        this.setState(state => ({value: this.props.value,
          inputValue: this.props.value,
          selection: findSelection(this.props.value, this.props.options),
          startComplete: true, endComplete: true, showError: false,
          revision: state.revision + 1}), this.publishValidation);
      }
    } else if(previous.options !== this.props.options &&
        this.state.selection !== 'custom') {
      this.setState({selection:
        findSelection(this.state.value, this.props.options)});
    }
    if(previous.boundsRequired !== this.props.boundsRequired) {
      this.publishValidation();
    }
  }

  public render(): JSX.Element {
    const {label, value, options, orientation, labelPosition, boundsRequired,
      readOnly, disabled, validation: supplied, onChange, onValidationChange,
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
    const rootStyle: React.CSSProperties = {};
    if(horizontal) {
      rootStyle.maxHeight = `${this.state.customHeight}px`;
    }
    const error = (target: DateRangeValidation.Target) => visibleError &&
      (validation.target === target ||
        validation.target === Target.START_AND_END);
    return <div {...rest} className={[css(STYLES.container,
        horizontal && STYLES.horizontal), className].join(' ')}
        style={{...rootStyle, ...style}}>
      <SelectGroup id={`${identifier}-preset`} label={label}
        controls={`${identifier}-custom`} selection={this.state.selection}
        options={options} horizontal={horizontal} inline={inline}
        labelInline={labelInline}
        readOnly={readOnly} disabled={disabled} onChange={this.onSelect}/>
      <CustomDates id={`${identifier}-custom`} custom={custom}
          horizontal={horizontal} inline={inline}
          labelInline={labelInline}
          hasLabel={Boolean(label)} startId={`${identifier}-start`}
          endId={`${identifier}-end`} errorId={errorId} label={label}
          validation={validation} onHeight={this.onHeight}
          start={<BoundDate key={`start-${this.state.revision}`}
            id={`${identifier}-start`} label='Start date'
            value={this.state.inputValue.start} error={error(Target.START)}
            errorId={errorId} readOnly={readOnly}
            disabled={disabled || !custom}
            onInput={this.onStartInput} onCommit={this.onCommit}/>}
          end={<BoundDate key={`end-${this.state.revision}`}
            id={`${identifier}-end`} label='End date'
            value={this.state.inputValue.end} error={error(Target.END)}
            errorId={errorId} readOnly={readOnly}
            disabled={disabled || !custom}
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
    const value = new DateRange(option.startDate, option.endDate);
    this.setState(state => ({value, inputValue: value, selection,
      startComplete: true, endComplete: true, showError: false,
      revision: state.revision + 1}), () => {
      this.publishValidation();
      if(this.getValidation().valid) {
        this.submission = value;
        this.props.onChange?.(value);
      }
    });
  };

  private onStartInput = (start: Beam.Date, startComplete: boolean) => {
    this.setState(state => ({value: new DateRange(start, state.value.end),
      startComplete, showError: false}), this.publishValidation);
  };

  private onEndInput = (end: Beam.Date, endComplete: boolean) => {
    this.setState(state => ({value: new DateRange(state.value.start, end),
      endComplete, showError: false}), this.publishValidation);
  };

  private onCommit = () => {
    this.setState({showError: true}, () => {
      this.publishValidation();
      if(this.getValidation().valid &&
          !isDateRangeEqual(this.state.value, this.submission)) {
        this.submission = this.state.value;
        this.props.onChange?.(this.state.value);
      }
    });
  };

  private onHeight = (customHeight: number) => {
    if(this.state.customHeight !== customHeight) {
      this.setState({customHeight});
    }
  };

  private static nextIdentifier = 0;
  private identifier: string;
  private submission: DateRange;
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
  hasLabel: boolean;
  startId: string;
  endId: string;
  errorId: string;
  label: string;
  validation: DateRangeValidation;
  start: React.ReactNode;
  end: React.ReactNode;
  onHeight: (height: number) => void;
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
    this.start = React.createRef<HTMLDivElement>();
    this.label = React.createRef<HTMLLabelElement>();
    this.error = React.createRef<HTMLDivElement>();
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
    this.props.onHeight(this.element.current.getBoundingClientRect().height);
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    if(this.frame !== null) {
      window.cancelAnimationFrame(this.frame);
    }
  }

  public render(): JSX.Element {
    const props = this.props;
    const rows: React.CSSProperties = {display: 'grid'};
    let labelPadding = '10px';
    if(props.labelInline) {
      labelPadding = '0';
      if(props.hasLabel) {
        labelPadding = '24px';
      }
    }
    if(props.horizontal) {
      labelPadding = '0';
    }
    let endLabelStyle: React.CSSProperties = {};
    if(props.horizontal) {
      rows.alignItems = 'center';
      rows.columnGap = '8px';
      rows.gridTemplateColumns = 'max-content 150px max-content 150px';
      endLabelStyle = {marginInlineStart: '10px'};
    } else if(props.inline) {
      rows.gridTemplateColumns =
        'calc(50px + (100cqw - 246px) * (80 / 138)) minmax(0, 1fr)';
      rows.alignItems = 'center';
      rows.gap = '10px 8px';
      rows.paddingTop = '10px';
    } else {
      rows.gridTemplateColumns = 'minmax(0, 1fr)';
      rows.gap = '12px';
      rows.paddingTop = '18px';
      endLabelStyle = {marginTop: '8px'};
    }
    const style: React.CSSProperties = {maxHeight: this.state.maximum,
      flex: '0 0 auto'};
    if(!props.custom) {
      style.visibility = 'hidden';
      style.overflow = 'clip';
    } else if(!props.horizontal && !props.inline) {
      style.transition = 'max-height 200ms ease-in-out';
      style.overflow = 'clip';
    }
    return <div id={props.id} ref={this.element} style={style}
        aria-hidden={!props.custom}>
      <div ref={this.content}>
        <div style={rows}>
          <label ref={this.label} htmlFor={props.startId}
            style={{paddingInlineStart: labelPadding, boxSizing: 'border-box'}}>
            Start
          </label>
          <div ref={this.start}>{props.start}</div>
          <label htmlFor={props.endId} style={{paddingInlineStart: labelPadding,
              boxSizing: 'border-box', ...endLabelStyle}}>
            End
          </label>
          {props.end}
        </div>
        <div ref={this.error}>
          <Error id={props.errorId} label={props.label}
            inline={props.inline} validation={props.validation}/>
        </div>
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
      const dateHeight = this.start.current.getBoundingClientRect().height;
      const errorHeight = this.error.current.getBoundingClientRect().height;
      if(this.props.horizontal) {
        maximum = dateHeight + errorHeight;
      } else if(this.props.inline) {
        maximum = 2 * dateHeight + errorHeight + 20;
      } else {
        const labelHeight = this.label.current.getBoundingClientRect().height;
        maximum = 2 * (labelHeight + dateHeight) + errorHeight + 62;
      }
    }
    if(this.state.maximum !== maximum) {
      this.setState({maximum});
    }
  };

  private element: React.RefObject<HTMLDivElement>;
  private content: React.RefObject<HTMLDivElement>;
  private start: React.RefObject<HTMLDivElement>;
  private label: React.RefObject<HTMLLabelElement>;
  private error: React.RefObject<HTMLDivElement>;
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
    const labelStyle: React.CSSProperties = {paddingInlineStart: '10px'};
    if(props.labelInline) {
      labelStyle.paddingInlineStart = 0;
    }
    if(props.label) {
      if(props.horizontal) {
        style.display = 'flex';
        style.alignItems = 'center';
        style.gap = '8px';
        style.flex = '0 0 auto';
        selectStyle.width = '150px';
      } else if(props.inline) {
        style.display = 'grid';
        style.alignItems = 'center';
        style.gridTemplateColumns =
          'calc(50px + (100cqw - 246px) * (80 / 138)) minmax(0, 1fr)';
        style.gap = '8px';
      } else {
        style.display = 'flex';
        style.flexDirection = 'column';
        style.gap = '12px';
      }
    }
    return <div style={style}>
      {props.label && <label htmlFor={props.id} style={labelStyle}>
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

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%',
    font: '400 14px Roboto, system-ui, sans-serif', color: '#333333'},
  horizontal: {display: 'flex', alignItems: 'flex-start', gap: '18px'}
});
