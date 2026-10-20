import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DayOfMonthInput, DayOfWeekSelect, InputErrorMessage, InputGroup,
  IntervalInput, MonthSelect } from '../../components';
import { DayOfMonth, Interval, ValidationError, Weekday } from '../../models';
import { DayOfYear, Recurrence } from './recurrence';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange' | 'onInput'> {

  /** The recurrence to edit. */
  value: Recurrence;

  /** Whether the recurrence can only be viewed. */
  readOnly?: boolean;

  /** Whether interaction is disabled. */
  disabled?: boolean;

  /** Reports a selection or numeric value committed by the user. */
  onChange?: (value: Recurrence) => void;

  /** Reports the validity of the current draft. */
  onValidationChange?: (validation: InputGroup.ValidationState) => void;
}

interface State {
  value: Recurrence;
}

/** Edits a report's daily, weekly, monthly or yearly recurrence. */
export class RecurrenceInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {value: props.value};
    this.weekday = Weekday.MONDAY;
    this.day = new DayOfMonth(DayOfMonth.Position.FIRST,
      DayOfMonth.DayType.DAY);
    this.month = Beam.Date.Month.JANUARY;
    this.isIntervalCommitted = true;
    this.remember(props.value);
  }

  public render(): JSX.Element {
    const {value, readOnly, disabled, onChange, onValidationChange,
      className, ...attributes} = this.props;
    const interval = this.state.value.interval;
    const intervalError = this.getIntervalError();
    const dayError = this.getDayError();
    return <div {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <div className={css(STYLES.content)}>
        <InputGroup label='Every'
            validation={{valid: !intervalError, error: intervalError}}
            errorMessage={<InputErrorMessage label='Interval'
              value='interval' error={intervalError}/>}>
          <IntervalInput value={interval} readOnly={readOnly}
            disabled={disabled} required onInput={this.onIntervalInput}
            onBlur={this.onIntervalBlur} onKeyDown={this.onIntervalKeyDown}
            onChange={this.onIntervalChange}/>
        </InputGroup>
        {interval.unit === Interval.Unit.WEEK &&
          <InputGroup label='On'>
            <DayOfWeekSelect value={this.weekday} readOnly={readOnly}
              disabled={disabled} onChange={this.onWeekdayChange}/>
          </InputGroup>}
        {(interval.unit === Interval.Unit.MONTH ||
            interval.unit === Interval.Unit.YEAR) &&
          <InputGroup label='On'
              validation={{valid: !dayError, error: dayError}}
              errorMessage={<InputErrorMessage label='Day' value='day'
                error={dayError}/>}>
            <DayOfMonthInput value={this.day} readOnly={readOnly}
              disabled={disabled} onInput={this.onDayInput}
              onChange={this.onDayChange}/>
          </InputGroup>}
        {interval.unit === Interval.Unit.YEAR &&
          <InputGroup label='Of'>
            <MonthSelect value={this.month} readOnly={readOnly}
              disabled={disabled} onChange={this.onMonthChange}/>
          </InputGroup>}
      </div>
    </div>;
  }

  public componentDidMount(): void {
    this.publishValidation();
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value &&
        this.props.value !== this.state.value) {
      this.remember(this.props.value);
      this.isIntervalCommitted = true;
      this.setState({value: this.props.value}, this.publishValidation);
    }
  }

  private remember(value: Recurrence): void {
    if(value.interval.unit === Interval.Unit.WEEK) {
      this.weekday = value.occurrence as Weekday;
    } else if(value.interval.unit === Interval.Unit.MONTH) {
      this.day = value.occurrence as DayOfMonth;
    } else if(value.interval.unit === Interval.Unit.YEAR) {
      const occurrence = value.occurrence as DayOfYear;
      this.day = occurrence.dayOfMonth;
      this.month = occurrence.month;
    }
  }

  private getIntervalError(): ValidationError {
    if(this.state.value.interval.count == null) {
      return ValidationError.REQUIRED;
    }
    return ValidationError.NONE;
  }

  private getDayError(): ValidationError {
    const unit = this.state.value.interval.unit;
    if((unit === Interval.Unit.MONTH || unit === Interval.Unit.YEAR) &&
        this.day.position === DayOfMonth.Position.DAY &&
        this.day.day == null) {
      return ValidationError.REQUIRED;
    }
    return ValidationError.NONE;
  }

  private publishValidation = () => {
    const error = this.getIntervalError() || this.getDayError();
    this.props.onValidationChange?.({valid: !error, error});
  };

  private update(interval: Interval, commit: boolean): void {
    const occurrence = (() => {
      if(interval.unit === Interval.Unit.WEEK) {
        return this.weekday;
      } else if(interval.unit === Interval.Unit.MONTH) {
        return this.day;
      } else if(interval.unit === Interval.Unit.YEAR) {
        return new DayOfYear(this.day, this.month);
      }
      return null;
    })();
    const value = new Recurrence(interval, occurrence);
    this.setState({value}, () => {
      this.publishValidation();
      if(commit) {
        this.props.onChange?.(value);
      }
    });
  }

  private onIntervalInput = (event: React.FormEvent<HTMLDivElement>) => {
    const input = event.target as HTMLInputElement;
    if(this.props.readOnly || this.props.disabled ||
        input.tagName !== 'INPUT') {
      return;
    }
    const text = input.value.replace(/[^0-9]/g, '');
    let count: number = null;
    if(text !== '') {
      count = Number(text);
    }
    this.isIntervalCommitted = false;
    this.update(new Interval(count, this.state.value.interval.unit), false);
  };

  private onIntervalChange = (interval: Interval) => {
    if(!this.props.readOnly && !this.props.disabled) {
      this.isIntervalCommitted = true;
      this.update(interval, true);
    }
  };

  private onIntervalBlur = (event: React.FocusEvent<HTMLDivElement>) => {
    if((event.target as HTMLElement).tagName === 'INPUT' &&
        !this.isIntervalCommitted) {
      this.onIntervalChange(this.state.value.interval);
    }
  };

  private onIntervalKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    if(event.key === 'Enter' && !event.nativeEvent.isComposing &&
        !this.props.readOnly && !this.props.disabled &&
        (event.target as HTMLElement).tagName === 'INPUT') {
      event.preventDefault();
      const input = event.target as HTMLInputElement;
      input.blur();
      input.focus();
    }
  };

  private onWeekdayChange = (weekday: Weekday) => {
    if(!this.props.readOnly && !this.props.disabled) {
      this.weekday = weekday;
      this.update(this.state.value.interval, true);
    }
  };

  private onDayInput = (day: DayOfMonth) => {
    if(!this.props.readOnly && !this.props.disabled) {
      this.day = day;
      this.update(this.state.value.interval, false);
    }
  };

  private onDayChange = (day: DayOfMonth) => {
    if(!this.props.readOnly && !this.props.disabled) {
      this.day = day;
      this.update(this.state.value.interval, true);
    }
  };

  private onMonthChange = (month: Beam.Date.Month) => {
    if(!this.props.readOnly && !this.props.disabled) {
      this.month = month;
      this.update(this.state.value.interval, true);
    }
  };

  private weekday: Weekday;
  private day: DayOfMonth;
  private month: Beam.Date.Month;
  private isIntervalCommitted: boolean;
}

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%', minWidth: 0},
  content: {
    display: 'flex',
    flexDirection: 'column',
    '@container (width < 384px)': {gap: '20px'},
    '@container (min-width: 384px)': {gap: '10px'}
  }
});
