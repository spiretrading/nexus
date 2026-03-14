import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { IntegerInput } from './integer_input';

interface Properties {

  /** The value to display in the field. */
  value?: Beam.Duration;

  /** The largest value the hours field can hold. */
  maxHourValue?: number;

  /** The smallest value the hours field can hold. */
  minHourValue?: number;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** Determines if the component is in an error state. */
  error?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value?: Beam.Duration) => void;
}

/** A component that displays a duration. */
export function DurationInput(props: Properties): JSX.Element {
  const hours = React.useRef(props.value?.split().hours);
  const minutes = React.useRef(props.value?.split().minutes);
  const seconds = React.useRef(props.value?.split().seconds);
  const split = props.value?.split();
  hours.current = split?.hours;
  minutes.current = split?.minutes;
  seconds.current = split?.seconds;
  const onchange = (h?: number, m?: number, s?: number) => {
    if(h != null && m != null && s != null) {
      props.onChange?.(Beam.Duration.HOUR.multiply(h).add(
        Beam.Duration.MINUTE.multiply(m)).add(
        Beam.Duration.SECOND.multiply(s)));
    } else if(h == null && m == null && s == null) {
      props.onChange?.(undefined);
    }
  };
  const onHoursChange = (value?: number) => {
    onchange(value, minutes.current, seconds.current);
  };
  const onMinutesChange = (value?: number) => {
    onchange(hours.current, value, seconds.current);
  };
  const onSecondsChange = (value?: number) => {
    onchange(hours.current, minutes.current, value);
  };
  const separatorStyle = props.value ? undefined : {color: '#8C8C8C'};
  return (
    <div className={css(STYLES.container,
        props.disabled && STYLES.containerDisabled,
        props.error && STYLES.containerError,
        props.readonly && STYLES.containerReadonly)}>
      <IntegerInput
        aria-label='Hours' placeholder='hh'
        min={props.minHourValue ?? 0} max={props.maxHourValue ?? 99}
        value={split?.hours}
        readOnly={props.readonly}
        disabled={props.disabled}
        onChange={onHoursChange}
        style={STYLE.hoursInput}
        leadingZeros={2}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        :
      </span>
      <IntegerInput
        aria-label='Minutes' placeholder='mm'
        min={0} max={59}
        value={split?.minutes}
        readOnly={props.readonly}
        disabled={props.disabled}
        onChange={onMinutesChange}
        style={STYLE.minutesInput}
        leadingZeros={2}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        :
      </span>
      <IntegerInput
        aria-label='Seconds' placeholder='ss'
        min={0} max={59}
        value={split?.seconds}
        readOnly={props.readonly}
        disabled={props.disabled}
        onChange={onSecondsChange}
        style={STYLE.secondsInput}
        leadingZeros={2}/>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    backgroundColor: '#FFFFFF',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    color: '#000000',
    fontSize: '1rem',
    fontFamily: "'Roboto', system-ui, sans-serif",
    padding: '3px 9px',
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
    boxSizing: 'border-box',
    height: '34px',
    ':hover': {
      borderColor: '#684BC7'
    },
    ':focus-within': {
      borderColor: '#684BC7'
    }
  },
  containerDisabled: {
    opacity: 0.4,
    cursor: 'not-allowed',
    pointerEvents: 'none',
    ':hover': {
      borderColor: '#C8C8C8'
    },
    ':focus-within': {
      borderColor: '#C8C8C8'
    }
  },
  containerReadonly: {
    borderColor: 'transparent',
    ':hover': {
      borderColor: 'transparent'
    },
    ':focus-within': {
      borderColor: 'transparent'
    }
  },
  containerError: {
    borderColor: '#E63F44',
    ':hover': {
      borderColor: '#E63F44'
    },
    ':focus-within': {
      borderColor: '#E63F44'
    }
  },
  separator: {
    width: '0.625rem',
    textAlign: 'center',
    userSelect: 'none'
  }
});

const STYLE = {
  hoursInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1rem'
  } as React.CSSProperties,
  minutesInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.5rem'
  } as React.CSSProperties,
  secondsInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1rem'
  } as React.CSSProperties
};
