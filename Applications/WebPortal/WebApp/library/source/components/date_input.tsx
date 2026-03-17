import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { IntegerInput } from '..';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The value to display in the field. */
  value?: Beam.Date;

  /** Determines if the component is readonly. */
  readOnly?: boolean;

  /** Determines if the component is disabled. */
  disabled?: boolean;

  /** Determines if the component is in an error state. */
  error?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value?: Beam.Date) => void;
}

/** A component that displays a date. */
export function DateInput({id, className, value, readOnly, disabled, error,
    onChange, ...rest}: Properties): JSX.Element {
  const yearRef = React.useRef(value?.year);
  const monthRef = React.useRef(value?.month);
  const dayRef = React.useRef(value?.day);
  const prevValue = React.useRef(value);
  if(value !== prevValue.current) {
    if(value != null ||
        (yearRef.current == null && monthRef.current == null &&
          dayRef.current == null)) {
      yearRef.current = value?.year;
      monthRef.current = value?.month;
      dayRef.current = value?.day;
    }
    prevValue.current = value;
  }
  const onchange = (y?: number, m?: number, d?: number) => {
    yearRef.current = y;
    monthRef.current = m;
    dayRef.current = d;
    if(y != null && m != null && d != null) {
      onChange?.(new Beam.Date(y, m, d));
    } else if(y == null && m == null && d == null) {
      onChange?.(undefined);
    }
  };
  const onYearChange = (value?: number) => {
    onchange(value, monthRef.current, dayRef.current);
  };
  const onMonthChange = (value?: number) => {
    onchange(yearRef.current, value, dayRef.current);
  };
  const onDayChange = (value?: number) => {
    onchange(yearRef.current, monthRef.current, value);
  };
  const hasValue = yearRef.current != null || monthRef.current != null ||
    dayRef.current != null;
  const separatorStyle = hasValue ? undefined : {color: '#8C8C8C'};
  return (
    <div {...rest} className={[css(STYLES.container,
        disabled && STYLES.containerDisabled, error && STYLES.containerError,
        readOnly && STYLES.containerReadonly),
        className].filter(Boolean).join(' ')}>
      <IntegerInput
        id={id}
        aria-label='Year' placeholder='YYYY'
        min={0} max={9999}
        value={yearRef.current}
        readOnly={readOnly}
        disabled={disabled}
        onChange={onYearChange}
        style={STYLE.yearInput}
        leadingZeros={4}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        -
      </span>
      <IntegerInput
        aria-label='Month' placeholder='MM'
        min={1} max={12}
        value={monthRef.current}
        readOnly={readOnly}
        disabled={disabled}
        onChange={onMonthChange}
        style={STYLE.monthInput}
        leadingZeros={2}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        -
      </span>
      <IntegerInput
        aria-label='Day' placeholder='DD'
        min={1} max={31}
        value={dayRef.current}
        readOnly={readOnly}
        disabled={disabled}
        onChange={onDayChange}
        style={STYLE.dayInput}
        leadingZeros={2}/>
    </div>);
}

type DateLabelProperties = Omit<Properties, 'readOnly' | 'onChange'>;

/** A read-only date display. */
export function DateLabel(props: DateLabelProperties): JSX.Element {
  return <DateInput value={props.value} readOnly/>;
}

const STYLES = StyleSheet.create({
  container: {
    backgroundColor: '#FFFFFF',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    color: '#000000',
    fontSize: '0.875rem',
    fontFamily: "'Roboto', system-ui, sans-serif",
    padding: '9px 10px',
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
    width: '0.45rem',
    textAlign: 'center',
    userSelect: 'none'
  }
});

const STYLE: Record<string, React.CSSProperties> = {
  dayInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.45rem'
  },
  monthInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.8rem'
  },
  yearInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '2.6rem'
  }
};
