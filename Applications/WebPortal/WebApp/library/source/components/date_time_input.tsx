import * as Beam from 'beam';
import * as React from 'react';
import { DateInput } from './date_input';
import { TimeOfDayInput } from './time_of_day_input';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The value to display. */
  value?: Beam.DateTime;

  /** Determines if the component is readonly. */
  readOnly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.DateTime) => void;
}

const STYLE: React.CSSProperties = {
  display: 'flex',
  flexDirection: 'column',
  gap: '10px'
};

/** A component that displays both date and time. */
export function DateTimeInput(props: Properties): JSX.Element {
  const {id, style, value: supplied, readOnly, disabled, onChange,
    ...attributes} = props;
  const value = supplied ?? Beam.DateTime.now();
  const onDateChange = (date?: Beam.Date) => {
    if(date != null) {
      onChange?.(new Beam.DateTime(date, value.timeOfDay));
    }
  };
  const onTimeChange = (time?: Beam.Duration) => {
    if(time != null) {
      onChange?.(new Beam.DateTime(value.date, time));
    }
  };
  return (
    <div {...attributes} style={{...STYLE, ...style}}>
      <DateInput
        id={id}
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        value={value.date}
        readOnly={readOnly}
        disabled={disabled}
        onChange={onDateChange}/>
      <TimeOfDayInput
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        value={value.timeOfDay}
        readOnly={readOnly}
        disabled={disabled}
        onChange={onTimeChange}/>
    </div>);
}

