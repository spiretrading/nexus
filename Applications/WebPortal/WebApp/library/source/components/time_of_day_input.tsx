import * as Beam from 'beam';
import * as React from 'react';
import { Select } from './select';
import { DurationInput } from './duration_input';

enum Period {
  AM,
  PM
}

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The value as a 24-hour time of day. */
  value?: Beam.Duration;

  /** Determines if the component is readOnly. */
  readOnly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** Determines if the component is in an error state. */
  error?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value?: Beam.Duration) => void;
}

/** A component that displays a time of day in 12-hour format with AM/PM. */
export function TimeOfDayInput({className, style, value, readOnly, disabled,
    error, onChange, ...rest}: Properties): JSX.Element {
  const [period, setPeriod] = React.useState(() => getPeriod(value));
  const [displayedTime, setDisplayedTime] =
    React.useState(() => to12Hour(value));
  const prevValue = React.useRef(value);
  if(value !== prevValue.current &&
      !(value?.equals(prevValue.current))) {
    prevValue.current = value;
    setPeriod(getPeriod(value));
    setDisplayedTime(to12Hour(value));
  }
  const onTimeChange = (time?: Beam.Duration) => {
    if(time != null) {
      setDisplayedTime(time);
      onChange?.(to24Hour(time, period));
    } else {
      onChange?.(undefined);
    }
  };
  const onPeriodChange = (v: string) => {
    const newPeriod = parseInt(v) as Period;
    setPeriod(newPeriod);
    if(displayedTime != null) {
      onChange?.(to24Hour(displayedTime, newPeriod));
    }
  };
  return (
    <div {...rest} className={className}
        style={{...STYLE.wrapper, ...style}}>
      <DurationInput
        style={{flex: 1}}
        value={displayedTime}
        readOnly={readOnly}
        disabled={disabled}
        error={error}
        maxHourValue={12}
        minHourValue={1}
        onChange={onTimeChange}/>
      <Select
          onChange={onPeriodChange}
          value={String(period)}
          readOnly={readOnly}
          disabled={disabled}
          style={STYLE.select}>
        <option value={String(Period.AM)}>AM</option>
        <option value={String(Period.PM)}>PM</option>
      </Select>
    </div>);
}

function getPeriod(value?: Beam.Duration): Period {
  if(value == null) {
    return Period.AM;
  }
  const hours = value.split().hours;
  if(hours >= 12 && hours < 24) {
    return Period.PM;
  }
  return Period.AM;
}

function to12Hour(value?: Beam.Duration): Beam.Duration | undefined {
  if(value == null) {
    return undefined;
  }
  const {hours, minutes, seconds} = value.split();
  let h: number;
  if(hours === 0 || hours === 24) {
    h = 12;
  } else if(hours > 12) {
    h = hours - 12;
  } else {
    h = hours;
  }
  return Beam.Duration.HOUR.multiply(h).add(
    Beam.Duration.MINUTE.multiply(minutes)).add(
    Beam.Duration.SECOND.multiply(seconds));
}

function to24Hour(displayedTime: Beam.Duration, period: Period): Beam.Duration {
  const {hours, minutes, seconds} = displayedTime.split();
  let h: number;
  if(period === Period.PM) {
    h = hours === 12 ? 12 : hours + 12;
  } else {
    h = hours === 12 ? 0 : hours;
  }
  return Beam.Duration.HOUR.multiply(h).add(
    Beam.Duration.MINUTE.multiply(minutes)).add(
    Beam.Duration.SECOND.multiply(seconds));
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    display: 'flex',
    flexDirection: 'row'
  },
  select: {
    marginLeft: '10px',
    width: '64px'
  }
};
