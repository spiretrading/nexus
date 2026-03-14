import * as Beam from 'beam';
import * as React from 'react';
import { DateInput } from './date_input';
import { TimeOfDayInput } from './time_of_day_input';

interface Properties {

  /** The value to display. */
  value?: Beam.DateTime;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.DateTime) => void;
}

const STYLE: React.CSSProperties = {
  display: 'flex',
  flexDirection: 'column'
};

/** A component that displays both date and time. */
export function DateTimeInput(props: Properties): JSX.Element {
  const value = props.value ?? Beam.DateTime.now();
  const onDateChange = (date?: Beam.Date) => {
    if(date != null) {
      props.onChange?.(new Beam.DateTime(date, value.timeOfDay));
    }
  };
  const onTimeChange = (time?: Beam.Duration) => {
    if(time != null) {
      props.onChange?.(new Beam.DateTime(value.date, time));
    }
  };
  return (
    <div style={STYLE}>
      <DateInput
        value={value.date}
        readonly={props.readonly}
        disabled={props.disabled}
        onChange={onDateChange}/>
      <div style={{height: '10px'}}/>
      <TimeOfDayInput
        value={value.timeOfDay}
        readonly={props.readonly}
        disabled={props.disabled}
        onChange={onTimeChange}/>
    </div>);
}

