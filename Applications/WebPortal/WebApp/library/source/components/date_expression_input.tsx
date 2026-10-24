import * as Beam from 'beam';
import * as React from 'react';
import { DateExpression, DateOffset } from '../models/date_expression';
import { DayOfMonth } from '../models/day_of_month';
import { Weekday } from '../models/date_rule';
import { DateInput } from './date_input';
import { DayOfMonthInput } from './day_of_month_input';
import { DayOfWeekSelect } from './day_of_week_select';
import { IntegerInput } from './integer_input';
import { Select } from './select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onInput' | 'onChange'> {

  /** The expression to edit, including incomplete drafts. */
  value: DateExpression;

  /** Whether the value can only be viewed. */
  readOnly?: boolean;

  /** Whether interaction is disabled. */
  disabled?: boolean;

  /** Reports a draft and whether all fixed-date segments are complete. */
  onInput?: (value: DateExpression, complete?: boolean) => void;

  /** Reports a commit and whether all fixed-date segments are complete. */
  onChange?: (value: DateExpression, complete?: boolean) => void;
}

interface State {
  value: DateExpression;
}

/** Edits a fixed date or an expression relative to a reference date. */
export class DateExpressionInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {value: props.value};
    this.values = new Map<DateExpression.Mode, DateExpression>([
      [DateExpression.Mode.FIXED,
        {mode: DateExpression.Mode.FIXED, date: Beam.Date.today()}],
      [DateExpression.Mode.DAY_OFFSET, {mode: DateExpression.Mode.DAY_OFFSET,
        date: {value: 0, unit: DateOffset.Unit.DAY}}],
      [DateExpression.Mode.WEEKDAY, {mode: DateExpression.Mode.WEEKDAY,
        date: {day: Weekday.MONDAY,
          offset: {value: 0, unit: DateOffset.Unit.WEEK}}}],
      [DateExpression.Mode.DAY_OF_MONTH,
        {mode: DateExpression.Mode.DAY_OF_MONTH,
          date: {day: new DayOfMonth(DayOfMonth.Position.FIRST,
            Weekday.MONDAY), offset: {value: 0, unit: DateOffset.Unit.MONTH}}}]
    ]);
    this.directions = new Map();
    this.dateElement = React.createRef<HTMLDivElement>();
    this.dateKey = 0;
    this.isDirty = false;
    this.store(props.value);
  }

  public render(): JSX.Element {
    const {id, value: ignored, readOnly, disabled, onInput, onChange,
      onKeyDown, style, ...rest} = this.props;
    const value = this.state.value;
    const shared = {readOnly, disabled,
      'aria-invalid': rest['aria-invalid'],
      'aria-describedby': rest['aria-describedby']};
    const offset = getOffset(value);
    const unit = (() => {
      if(value.mode === DateExpression.Mode.DAY_OFFSET) {
        return 'Day';
      } else if(value.mode === DateExpression.Mode.WEEKDAY) {
        return 'Week';
      }
      return 'Month';
    })();
    const noun = unit + (() => {
      if(Math.abs(offset?.value) === 1) {
        return '';
      }
      return 's';
    })();
    const fieldStyle: React.CSSProperties = {
      width: '100%', minWidth: 0, boxSizing: 'border-box'};
    return <div role='group' {...rest} onKeyDown={this.onKeyDown}
        style={{containerType: 'inline-size', display: 'flex',
          flexDirection: 'column', gap: '10px', width: '100%', minWidth: 0,
          ...style}}>
      <Select {...shared} id={id} aria-label='Date mode' value={value.mode}
          style={fieldStyle} onChange={this.onModeChange}>
        <option value={DateExpression.Mode.FIXED}>Fixed date</option>
        <option value={DateExpression.Mode.DAY_OFFSET}>Day offset</option>
        <option value={DateExpression.Mode.WEEKDAY}>Weekday</option>
        <option value={DateExpression.Mode.DAY_OF_MONTH}>Day of month</option>
      </Select>
      {value.mode === DateExpression.Mode.FIXED &&
        <div ref={this.dateElement} onInput={this.onDateInput}
            onBlur={this.onDateBlur}>
          <DateInput {...shared} key={this.dateKey} value={value.date}
            style={fieldStyle}
            onChange={this.onDateChange}/>
        </div>}
      {value.mode === DateExpression.Mode.WEEKDAY &&
        <DayOfWeekSelect {...shared} aria-label='Day of week'
          value={value.date.day} style={fieldStyle}
          onChange={this.onWeekdayChange}/>}
      {value.mode === DateExpression.Mode.DAY_OF_MONTH &&
        <DayOfMonthInput {...shared} value={value.date.day}
          onInput={this.onDayInput} onChange={this.onDayChange}/>}
      {offset && <div style={{display: 'grid', gap: '10px', minWidth: 0,
          gridTemplateColumns:
            'clamp(50px, calc(100cqw - 150px), 96px) minmax(0, 1fr)'}}
          onBlur={this.onOffsetBlur}>
        <IntegerInput {...shared} aria-label={`${unit} offset`}
          value={getMagnitude(offset.value)} min={0} inputMode='numeric'
          style={fieldStyle}
          onInput={this.onOffsetInput} onChange={this.onOffsetChange}/>
        <Select {...shared} aria-label={`${unit} offset direction`}
            value={this.directions.get(value.mode) || 'past'}
            style={fieldStyle}
            onChange={this.onDirectionChange}>
          <option value='past'>{noun} ago</option>
          <option value='future'>{noun} ahead</option>
        </Select>
      </div>}
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value &&
        this.props.value !== this.state.value) {
      if(this.props.value.mode === DateExpression.Mode.FIXED &&
          this.props.value.date == null) {
        ++this.dateKey;
      }
      this.store(this.props.value);
      this.isDirty = false;
      this.setState({value: this.props.value});
    }
  }

  private store(value: DateExpression): void {
    this.values.set(value.mode, value);
    const offset = getOffset(value)?.value;
    if(offset > 0) {
      this.directions.set(value.mode, 'future');
    } else if(offset < 0) {
      this.directions.set(value.mode, 'past');
    }
  }

  private update(value: DateExpression, isInput: boolean,
      complete?: boolean): void {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    this.store(value);
    this.setState({value});
    if(isInput) {
      this.props.onInput?.(value, complete);
    } else {
      this.props.onChange?.(value, complete);
    }
  }

  private updateOffset(count: number, isInput: boolean): void {
    const value = this.state.value;
    const offset = {...getOffset(value), value: count ?? null};
    if(offset.value != null &&
        this.directions.get(value.mode) !== 'future') {
      offset.value = -offset.value;
    }
    if(value.mode === DateExpression.Mode.DAY_OFFSET) {
      this.update({...value, date: offset}, isInput);
    } else if(value.mode === DateExpression.Mode.WEEKDAY ||
        value.mode === DateExpression.Mode.DAY_OF_MONTH) {
      this.update({...value, date: {...value.date, offset}} as DateExpression,
        isInput);
    }
  }

  private isDateEmpty(): boolean {
    return Array.from(this.dateElement.current.querySelectorAll('input')).
      every(input => input.value === '');
  }

  private readDate(): Beam.Date {
    const MINIMUM_YEAR = 1400;
    const MAXIMUM_YEAR = 9999;
    const parts = Array.from(this.dateElement.current.querySelectorAll('input'),
      input => input.value.replace(/[^0-9]/g, ''));
    if(parts.some(part => part === '')) {
      return null;
    }
    const [year, month, day] = parts.map(Number);
    const date = new Date(0);
    date.setUTCFullYear(year, month - 1, day);
    if(year < MINIMUM_YEAR || year > MAXIMUM_YEAR ||
        date.getUTCFullYear() !== year ||
        date.getUTCMonth() !== month - 1 || date.getUTCDate() !== day) {
      return null;
    }
    return new Beam.Date(year, month, day);
  }

  private onModeChange = (mode: string) => {
    this.isDirty = false;
    this.update(this.values.get(mode as DateExpression.Mode), false);
  };

  private onWeekdayChange = (day: Weekday) => {
    const value = this.state.value;
    if(value.mode === DateExpression.Mode.WEEKDAY) {
      this.update({...value, date: {...value.date, day}}, false);
    }
  };

  private onDayInput = (day: DayOfMonth) => {
    const value = this.state.value;
    if(value.mode === DateExpression.Mode.DAY_OF_MONTH) {
      this.update({...value, date: {...value.date, day}}, true);
    }
  };

  private onDayChange = (day: DayOfMonth) => {
    const value = this.state.value;
    if(value.mode === DateExpression.Mode.DAY_OF_MONTH) {
      this.update({...value, date: {...value.date, day}}, false);
    }
  };

  private onOffsetInput = (event: React.FormEvent<HTMLInputElement>) => {
    this.isDirty = true;
    const text = event.currentTarget.value.replace(/[^0-9]/g, '');
    let count: number = null;
    if(text !== '') {
      count = Number(text);
    }
    this.updateOffset(count, true);
  };

  private onOffsetChange = (count: number) => {
    this.isDirty = false;
    this.updateOffset(count, false);
  };

  private onOffsetBlur = (event: React.FocusEvent<HTMLDivElement>) => {
    if((event.target as HTMLElement).tagName === 'INPUT' && this.isDirty) {
      this.onOffsetChange(getMagnitude(getOffset(this.state.value).value));
    }
  };

  private onDirectionChange = (direction: string) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    this.directions.set(this.state.value.mode, direction);
    this.updateOffset(getMagnitude(getOffset(this.state.value).value), false);
  };

  private onDateInput = () => {
    this.isDirty = true;
    const date = this.readDate();
    this.update({mode: DateExpression.Mode.FIXED, date}, true,
      date != null || this.isDateEmpty());
  };

  private onDateChange = (date: Beam.Date) => {
    this.isDirty = false;
    this.update({mode: DateExpression.Mode.FIXED, date: date ?? null}, false,
      date != null || this.isDateEmpty());
  };

  private onDateBlur = () => {
    if(this.isDirty) {
      this.onDateChange(this.readDate());
    }
  };

  private onKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    this.props.onKeyDown?.(event);
    if(event.defaultPrevented || this.props.readOnly || this.props.disabled) {
      return;
    }
    if(event.key === 'Enter' && !event.nativeEvent.isComposing &&
        (event.target as HTMLElement).tagName === 'INPUT') {
      event.preventDefault();
      const input = event.target as HTMLInputElement;
      input.blur();
      input.focus();
    }
  };

  private values: Map<DateExpression.Mode, DateExpression>;
  private directions: Map<DateExpression.Mode, string>;
  private dateElement: React.RefObject<HTMLDivElement>;
  private dateKey: number;
  private isDirty: boolean;
}

function getOffset(value: DateExpression): DateOffset {
  if(value.mode === DateExpression.Mode.DAY_OFFSET) {
    return value.date;
  } else if(value.mode === DateExpression.Mode.WEEKDAY ||
      value.mode === DateExpression.Mode.DAY_OF_MONTH) {
    return value.date.offset;
  }
  return null;
}

function getMagnitude(value: number): number {
  if(value == null) {
    return null;
  }
  return Math.abs(value);
}
