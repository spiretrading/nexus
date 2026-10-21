import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DayOfMonth } from '../models/day_of_month';
import { Weekday } from '../models/date_rule';
import { IntegerInput } from './integer_input';
import { Select } from './select';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange' | 'onInput'> {

  /** The day of the month to edit. */
  value: DayOfMonth;

  /** Whether the value can only be viewed. */
  readOnly?: boolean;

  /** Whether interaction is disabled. */
  disabled?: boolean;

  /** Reports numeric drafts, including an empty or out-of-range day. */
  onInput?: (value: DayOfMonth) => void;

  /** Reports a selection or numeric value committed by blur or Enter. */
  onChange?: (value: DayOfMonth) => void;
}

interface State {
  dayNumber: number;
}

/** Edits an ordinal or numeric day within a month. */
export class DayOfMonthInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {dayNumber: (() => {
      if(props.value.position === DayOfMonth.Position.DAY) {
        return props.value.day as number;
      }
      return 1;
    })()};
    this.dayType = (() => {
      if(props.value.position === DayOfMonth.Position.DAY) {
        return Weekday.MONDAY;
      }
      return props.value.day as Weekday | DayOfMonth.DayType;
    })();
    this.isNumberCommitted = true;
  }

  public render(): JSX.Element {
    const {id, value, readOnly, disabled, onInput, onChange, onBlur, onKeyDown,
      style, ...rest} = this.props;
    const fieldStyle: React.CSSProperties = {
      flex: '1 1 0', width: '100%', minWidth: 0, boxSizing: 'border-box'};
    return <div role='group' {...rest}
        style={{containerType: 'inline-size', width: '100%', ...style}}
        onBlur={this.onBlur} onKeyDown={this.onKeyDown}>
      <div className={css(STYLES.fields)}>
        <Select id={id} aria-label='Position' value={value.position}
            readOnly={readOnly} disabled={disabled}
            style={fieldStyle}
            onChange={this.onPositionChange}>
          {Object.values(DayOfMonth.Position).map(position =>
            <option key={position} value={position}>
              {position[0] + position.slice(1).toLowerCase()}
            </option>)}
        </Select>
        {value.position === DayOfMonth.Position.DAY &&
          <IntegerInput aria-label='Day number' value={this.state.dayNumber}
            min={1} max={31} step={1} inputMode='numeric'
            aria-invalid={rest['aria-invalid']}
            aria-describedby={rest['aria-describedby']}
            readOnly={readOnly} disabled={disabled} style={fieldStyle}
            onInput={this.onNumberInput} onChange={this.onNumberChange}/>}
        {value.position !== DayOfMonth.Position.DAY &&
          <Select aria-label='Day type' value={value.day}
              readOnly={readOnly} disabled={disabled} style={fieldStyle}
              onChange={this.onDayTypeChange}>
            {[...Object.values(Weekday), ...Object.values(DayOfMonth.DayType)].
              map(day => <option key={day} value={day}>
                {day[0] + day.slice(1).toLowerCase()}
              </option>)}
          </Select>}
      </div>
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value) {
      this.updateDay(this.props.value);
    }
  }

  private updateDay(value: DayOfMonth): void {
    if(value.position === DayOfMonth.Position.DAY) {
      if(value.day !== this.state.dayNumber) {
        this.setState({dayNumber: value.day as number});
        this.isNumberCommitted = true;
      }
    } else {
      this.dayType = value.day as Weekday | DayOfMonth.DayType;
      this.isNumberCommitted = true;
    }
  }

  private onPositionChange = (value: string) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    const position = value as DayOfMonth.Position;
    const day = (() => {
      if(position === DayOfMonth.Position.DAY) {
        return this.state.dayNumber;
      }
      return this.dayType;
    })();
    this.props.onChange?.(new DayOfMonth(position, day));
  };

  private onDayTypeChange = (value: string) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    this.dayType = value as Weekday | DayOfMonth.DayType;
    this.props.onChange?.(new DayOfMonth(this.props.value.position,
      this.dayType));
  };

  private onNumberInput = (event: React.FormEvent<HTMLInputElement>) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    const text = event.currentTarget.value.replace(/[^0-9]/g, '');
    let dayNumber: number = null;
    if(text !== '') {
      dayNumber = Number(text);
    }
    this.setState({dayNumber});
    this.isNumberCommitted = false;
    this.props.onInput?.(new DayOfMonth(DayOfMonth.Position.DAY,
      dayNumber));
  };

  private onNumberChange = (value: number) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    let dayNumber = value ?? null;
    if(dayNumber != null) {
      dayNumber = Math.min(31, Math.max(1, dayNumber));
    }
    this.setState({dayNumber});
    this.isNumberCommitted = true;
    this.props.onChange?.(new DayOfMonth(DayOfMonth.Position.DAY,
      dayNumber));
  };

  private onBlur = (event: React.FocusEvent<HTMLDivElement>) => {
    if(this.props.value.position === DayOfMonth.Position.DAY &&
        (event.target as HTMLElement).tagName === 'INPUT' &&
        !this.isNumberCommitted) {
      this.onNumberChange(this.state.dayNumber);
    }
    this.props.onBlur?.(event);
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

  private dayType: Weekday | DayOfMonth.DayType;
  private isNumberCommitted: boolean;
}

const STYLES = StyleSheet.create({
  fields: {
    display: 'grid',
    gap: '10px',
    '@container (width < 246px)': {gridTemplateColumns: 'minmax(0, 1fr)'},
    '@container (min-width: 246px)': {
      gridTemplateColumns: '96px minmax(0, 1fr)'
    }
  }
});
