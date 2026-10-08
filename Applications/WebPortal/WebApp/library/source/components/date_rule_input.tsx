import * as Beam from 'beam';
import * as React from 'react';
import { DateRule, DateRuleType, DayOffsetDateRule, DayOfMonthDateRule,
  MonthBoundaryDateRule, SpecificDateRule, Weekday, WeekdayDateRule } from
  '../models';
import { DateInput } from './date_input';
import { IntegerInput } from './integer_input';
import { Select } from './select';

interface Properties {

  /** The identifier of the rule selector. */
  id?: string;

  /** The accessible label for this rule and its fields. */
  label: string;

  /** The rule being edited, including incomplete draft values. */
  value: DateRule;

  /** The calendar date used when selecting a new rule type. */
  reference: Beam.Date;

  /** Overrides the fixed date displayed while editing an incomplete bound. */
  dateValue?: Beam.Date;

  /** The resolved date used when switching rule types. */
  resolved?: Beam.Date;

  /** The selectable rule types. Omit to offer every type. */
  types?: readonly DateRuleType[];

  /** Whether to show the relative day, week, or month offset. */
  showOffset?: boolean;

  /** Whether the input has a validation error. */
  error?: boolean;

  /** The identifier of its validation message. */
  errorId?: string;

  /** Whether the value can only be viewed. */
  readOnly?: boolean;

  /** Whether interaction is disabled. */
  disabled?: boolean;

  /** Reports an edited rule. */
  onChange: (rule: DateRule) => void;

  /** Receives fixed-date drafts instead of onChange when supplied. */
  onInput?: (value: Beam.Date, complete: boolean) => void;

  /** Reports a fixed-date commit. */
  onCommit?: () => void;
}

enum OffsetDirection {
  BEFORE = 'Before',
  AFTER = 'After'
}

interface DateRuleInputState {
  direction: OffsetDirection;
  boundaryDirection: OffsetDirection;
}

/** Edits a fixed or relative calendar date rule. */
export class DateRuleInput extends React.Component<Properties,
    DateRuleInputState> {
  constructor(props: Properties) {
    super(props);
    const state = {direction: OffsetDirection.BEFORE,
      boundaryDirection: OffsetDirection.BEFORE};
    if(props.value.type !== DateRuleType.SPECIFIC_DATE &&
        props.value.offset > 0) {
      state.direction = OffsetDirection.AFTER;
    }
    if(props.value.type === DateRuleType.MONTH_BOUNDARY &&
        props.value.dayOffset > 0) {
      state.boundaryDirection = OffsetDirection.AFTER;
    }
    this.state = state;
  }

  public componentDidUpdate(previous: Properties): void {
    const rule = this.props.value;
    if(previous.value === rule) {
      return;
    }
    if(previous.value.type !== rule.type) {
      this.setState({direction: OffsetDirection.BEFORE,
        boundaryDirection: OffsetDirection.BEFORE});
    }
    if(rule.type !== DateRuleType.SPECIFIC_DATE && rule.offset) {
      this.setState({direction: getDirection(rule.offset)});
    }
    if(rule.type === DateRuleType.MONTH_BOUNDARY && rule.dayOffset) {
      this.setState({boundaryDirection: getDirection(rule.dayOffset)});
    }
  }

  public render(): JSX.Element {
    const props = this.props;
    const rule = props.value;
    const specific = rule.type === DateRuleType.SPECIFIC_DATE;
    const style: React.CSSProperties = {width: '100%', minWidth: 0,
      boxSizing: 'border-box'};
    const unit = (() => {
      if(rule.type === DateRuleType.DAY_OFFSET) {
        return 'Days';
      } else if(rule.type === DateRuleType.WEEKDAY) {
        return 'Weeks';
      }
      return 'Months';
    })();
    return <div role='group' aria-label={`${props.label} date rule`}
        style={{display: 'grid', gap: '8px', minWidth: 0}}>
      <Select id={props.id} aria-label={`${props.label} rule`}
          value={rule.type} style={style} readOnly={props.readOnly}
          disabled={props.disabled} onChange={this.onTypeChange}>
        {RULE_TYPES.filter(([type]) => !props.types ||
          props.types.includes(type)).map(([type, label]) =>
            <option key={type} value={type}>{label}</option>)}
      </Select>
      {specific && <BoundDate id={`${props.id}-date`}
        label={`${props.label} date`} value={(() => {
          if(props.dateValue !== undefined) {
            return props.dateValue;
          }
          return (rule as SpecificDateRule).date;
        })()}
        error={props.error} errorId={props.errorId} readOnly={props.readOnly}
        disabled={props.disabled} onInput={this.onDateInput}
        onCommit={props.onCommit}/>}
      {rule.type === DateRuleType.WEEKDAY &&
        <Select aria-label={`${props.label} weekday`} value={rule.day}
            style={style} readOnly={props.readOnly} disabled={props.disabled}
            onChange={this.onWeekdayChange}>
          {Object.values(Weekday).map(day =>
            <option value={day} key={day}>
              {day[0] + day.slice(1).toLowerCase()}
            </option>)}
        </Select>}
      {rule.type === DateRuleType.DAY_OF_MONTH &&
        <label style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
          Day
          <IntegerInput aria-label={`${props.label} day of month`}
            value={rule.day} min={1} max={31} step={1} inputMode='numeric'
            readOnly={props.readOnly} disabled={props.disabled}
            aria-invalid={props.error} aria-describedby={props.errorId}
            style={{width: '64px', minWidth: 0}} onChange={this.onDayChange}/>
        </label>}
      {rule.type === DateRuleType.MONTH_BOUNDARY &&
        <div style={{display: 'grid', gap: '6px',
            gridTemplateColumns: '40px minmax(0, 1.2fr) minmax(0, 1fr)'}}>
          <IntegerInput aria-label={`${props.label} boundary day offset`}
            value={getMagnitude(rule.dayOffset)} min={0} step={1}
            inputMode='numeric'
            readOnly={props.readOnly} disabled={props.disabled}
            aria-invalid={props.error} aria-describedby={props.errorId}
            style={{width: '100%', minWidth: 0}}
            onChange={this.onBoundaryOffsetChange}/>
          <Select aria-label={`${props.label} boundary direction`}
              value={this.state.boundaryDirection}
              style={{...style, paddingLeft: '6px', paddingRight: '20px',
                backgroundPosition: 'right 7px top 50%'}}
              readOnly={props.readOnly} disabled={props.disabled}
              onChange={this.onBoundaryDirectionChange}>
            <option value={OffsetDirection.BEFORE}>Days before</option>
            <option value={OffsetDirection.AFTER}>Days after</option>
          </Select>
          <Select aria-label={`${props.label} month boundary`}
              value={rule.boundary}
              style={{...style, paddingLeft: '6px', paddingRight: '20px',
                backgroundPosition: 'right 7px top 50%'}}
              readOnly={props.readOnly} disabled={props.disabled}
              onChange={this.onBoundaryChange}>
            <option value={MonthBoundaryDateRule.Boundary.FIRST}>
              First day
            </option>
            <option value={MonthBoundaryDateRule.Boundary.LAST}>
              Last day
            </option>
          </Select>
        </div>}
      {props.showOffset !== false &&
        rule.type !== DateRuleType.SPECIFIC_DATE && <div
          style={{display: 'flex', flexWrap: 'wrap', gap: '8px'}}>
        <IntegerInput aria-label={`${props.label} offset`}
          value={getMagnitude(rule.offset)}
          min={0} step={1} inputMode='numeric' readOnly={props.readOnly}
          disabled={props.disabled} aria-invalid={props.error}
          aria-describedby={props.errorId}
          style={{width: '64px', flex: '0 0 64px', minWidth: 0}}
          onChange={this.onCountChange}/>
        <Select aria-label={`${props.label} direction`}
            value={this.state.direction}
            style={{...style, flex: '1 1 116px', width: 'auto'}}
            readOnly={props.readOnly} disabled={props.disabled}
            onChange={this.onDirectionChange}>
          <option value={OffsetDirection.BEFORE}>{unit} ago</option>
          <option value={OffsetDirection.AFTER}>{unit} ahead</option>
        </Select>
      </div>}
    </div>;
  }

  private change(value: DateRule): void {
    if(!this.props.readOnly && !this.props.disabled) {
      this.props.onChange(value);
    }
  }

  private setOffset(offset: number): void {
    const rule = this.props.value;
    if(rule.type === DateRuleType.DAY_OFFSET) {
      this.change(new DayOffsetDateRule(offset));
    } else if(rule.type === DateRuleType.WEEKDAY) {
      this.change(new WeekdayDateRule(offset, rule.day));
    } else if(rule.type === DateRuleType.DAY_OF_MONTH) {
      this.change(new DayOfMonthDateRule(offset, rule.day));
    } else if(rule.type === DateRuleType.MONTH_BOUNDARY) {
      this.change(new MonthBoundaryDateRule(offset, rule.boundary,
        rule.dayOffset));
    }
  }

  private onDateInput = (date: Beam.Date, complete: boolean) => {
    if(this.props.onInput) {
      this.props.onInput(date, complete);
    } else {
      this.change(new SpecificDateRule(date));
    }
  };

  private onTypeChange = (type: string) => {
    if(type === DateRuleType.SPECIFIC_DATE) {
      this.change(new SpecificDateRule(
        this.props.resolved ?? this.props.reference));
    } else if(type === DateRuleType.DAY_OFFSET) {
      this.change(new DayOffsetDateRule(0));
    } else if(type === DateRuleType.WEEKDAY) {
      this.change(new WeekdayDateRule(0, Weekday.MONDAY));
    } else if(type === DateRuleType.DAY_OF_MONTH) {
      this.change(new DayOfMonthDateRule(0,
        (this.props.resolved ?? this.props.reference).day));
    } else if(type === DateRuleType.MONTH_BOUNDARY) {
      this.change(new MonthBoundaryDateRule(0,
        MonthBoundaryDateRule.Boundary.LAST, 0));
    }
  };

  private onWeekdayChange = (day: string) => {
    const rule = this.props.value;
    if(rule.type === DateRuleType.WEEKDAY) {
      this.change(new WeekdayDateRule(rule.offset, day as Weekday));
    }
  };

  private onDayChange = (day: number) => {
    const rule = this.props.value;
    if(rule.type === DateRuleType.DAY_OF_MONTH) {
      this.change(new DayOfMonthDateRule(rule.offset, day ?? null));
    }
  };

  private onBoundaryChange = (boundary: string) => {
    const rule = this.props.value;
    if(rule.type === DateRuleType.MONTH_BOUNDARY) {
      this.change(new MonthBoundaryDateRule(rule.offset,
        boundary as MonthBoundaryDateRule.Boundary, rule.dayOffset));
    }
  };

  private onBoundaryOffsetChange = (offset: number) => {
    const rule = this.props.value;
    if(rule.type === DateRuleType.MONTH_BOUNDARY) {
      this.change(new MonthBoundaryDateRule(rule.offset, rule.boundary,
        signedOffset(offset, this.state.boundaryDirection)));
    }
  };

  private onBoundaryDirectionChange = (value: string) => {
    const direction = value as OffsetDirection;
    const rule = this.props.value;
    if(rule.type === DateRuleType.MONTH_BOUNDARY) {
      this.setState({boundaryDirection: direction});
      this.change(new MonthBoundaryDateRule(rule.offset, rule.boundary,
        signedOffset(getMagnitude(rule.dayOffset), direction)));
    }
  };

  private onCountChange = (count: number) => {
    this.setOffset(signedOffset(count, this.state.direction));
  };

  private onDirectionChange = (value: string) => {
    const direction = value as OffsetDirection;
    if(this.props.value.type !== DateRuleType.SPECIFIC_DATE) {
      this.setState({direction});
      this.setOffset(signedOffset(
        getMagnitude(this.props.value.offset), direction));
    }
  };
}

function getDirection(offset: number): OffsetDirection {
  if(offset > 0) {
    return OffsetDirection.AFTER;
  }
  return OffsetDirection.BEFORE;
}

function getMagnitude(offset: number): number {
  if(offset == null) {
    return null;
  }
  return Math.abs(offset);
}

function signedOffset(count: number, direction: OffsetDirection): number {
  if(count == null) {
    return null;
  }
  if(direction === OffsetDirection.BEFORE) {
    return -count;
  }
  return count;
}

const RULE_TYPES: readonly [DateRuleType, string][] = [
  [DateRuleType.SPECIFIC_DATE, 'Specific date'],
  [DateRuleType.DAY_OFFSET, 'Day offset'],
  [DateRuleType.WEEKDAY, 'Weekday'],
  [DateRuleType.DAY_OF_MONTH, 'Day of month'],
  [DateRuleType.MONTH_BOUNDARY, 'Month boundary']
];

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
