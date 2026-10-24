import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { IntegerInput } from './integer_input';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The value to display in the field. */
  value?: Beam.Duration;

  /** The largest value the hours field can hold. */
  maxHourValue?: number;

  /** The smallest value the hours field can hold. */
  minHourValue?: number;

  /** Determines if the component is readonly. */
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

interface State {
  value: Beam.Duration;
  hours: number;
  minutes: number;
  seconds: number;
}

/** A component that displays a duration. */
export class DurationInput extends React.Component<Properties, State> {
  public static getDerivedStateFromProps(props: Properties, state: State):
      Partial<State> {
    if(props.value === state.value || props.value?.equals(state.value)) {
      return null;
    }
    const split = props.value?.split();
    return {value: props.value, hours: split?.hours, minutes: split?.minutes,
      seconds: split?.seconds};
  }

  constructor(props: Properties) {
    super(props);
    const split = props.value?.split();
    this.state = {value: props.value, hours: split?.hours,
      minutes: split?.minutes, seconds: split?.seconds};
  }

  public render(): JSX.Element {
    const {id, className, value, maxHourValue, minHourValue, readOnly, disabled,
      error, onChange, ...rest} = this.props;
    const isInvalid = error || rest['aria-invalid'] === true ||
      rest['aria-invalid'] === 'true';
    const separatorStyle = (() => {
      if(value) {
        return undefined;
      }
      return {color: '#8C8C8C'};
    })();
    return (
      <div {...rest} className={[css(STYLES.container,
          disabled && STYLES.containerDisabled,
          isInvalid && STYLES.containerError,
          readOnly && STYLES.containerReadonly),
          className].filter(Boolean).join(' ')}>
        <IntegerInput
          id={id}
          aria-invalid={rest['aria-invalid']}
          aria-describedby={rest['aria-describedby']}
          aria-label='Hours' placeholder='hh'
          min={minHourValue ?? 0} max={maxHourValue ?? 99}
          value={this.state.hours}
          readOnly={readOnly}
          disabled={disabled}
          onChange={this.onHoursChange}
          style={STYLE.hoursInput}
          leadingZeros={2}/>
        <span className={css(STYLES.separator)} style={separatorStyle}>
          :
        </span>
        <IntegerInput
          aria-label='Minutes' placeholder='mm'
          aria-invalid={rest['aria-invalid']}
          aria-describedby={rest['aria-describedby']}
          min={0} max={59}
          value={this.state.minutes}
          readOnly={readOnly}
          disabled={disabled}
          onChange={this.onMinutesChange}
          style={STYLE.minutesInput}
          leadingZeros={2}/>
        <span className={css(STYLES.separator)} style={separatorStyle}>
          :
        </span>
        <IntegerInput
          aria-label='Seconds' placeholder='ss'
          aria-invalid={rest['aria-invalid']}
          aria-describedby={rest['aria-describedby']}
          min={0} max={59}
          value={this.state.seconds}
          readOnly={readOnly}
          disabled={disabled}
          onChange={this.onSecondsChange}
          style={STYLE.secondsInput}
          leadingZeros={2}/>
      </div>);
  }

  private publish = () => {
    const {hours, minutes, seconds} = this.state;
    if(hours != null && minutes != null && seconds != null) {
      this.props.onChange?.(Beam.Duration.HOUR.multiply(hours).add(
        Beam.Duration.MINUTE.multiply(minutes)).add(
        Beam.Duration.SECOND.multiply(seconds)));
    } else if(hours == null && minutes == null && seconds == null) {
      this.props.onChange?.(undefined);
    }
  };

  private onHoursChange = (hours: number) => {
    this.setState({hours}, this.publish);
  };

  private onMinutesChange = (minutes: number) => {
    this.setState({minutes}, this.publish);
  };

  private onSecondsChange = (seconds: number) => {
    this.setState({seconds}, this.publish);
  };
}

const STYLES = StyleSheet.create({
  container: {
    backgroundColor: '#FFFFFF',
    borderWidth: '1px',
    borderStyle: 'solid',
    borderColor: '#C8C8C8',
    borderRadius: '1px',
    color: '#000000',
    fontSize: '0.875rem',
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
    width: '0.714em',
    textAlign: 'center',
    userSelect: 'none'
  }
});

const STYLE: Record<string, React.CSSProperties> = {
  hoursInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.142em'
  },
  minutesInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.714em'
  },
  secondsInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.142em'
  }
};
