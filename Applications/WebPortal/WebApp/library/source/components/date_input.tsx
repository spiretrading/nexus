import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { IntegerInput } from '..';

interface Properties {

  /** The value to display in the field. */
  value?: Beam.Date;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Determines if the component is in an error state. */
  error?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Date) => void;
}

/** A component that displays a date. */
export function DateInput(props: Properties): JSX.Element {
  const onYearChange = (year: number) => {
    const old = props.value ?? today();
    props.onChange?.(new Beam.Date(year, old.month, old.day));
  };
  const onMonthChange = (month: number) => {
    const old = props.value ?? today();
    props.onChange?.(new Beam.Date(old.year, month, old.day));
  };
  const onDayChange = (day: number) => {
    const old = props.value ?? today();
    props.onChange?.(new Beam.Date(old.year, old.month, day));
  };
  const separatorStyle = props.value ? undefined : {color: '#8C8C8C'};
  return (
    <div className={css(STYLES.container,
        props.error && STYLES.containerError,
        props.readonly && STYLES.containerReadonly)}>
      <IntegerInput
        aria-label='Year' placeholder='YYYY'
        min={0} max={9999}
        value={props.value?.year}
        readOnly={props.readonly}
        onChange={onYearChange}
        style={STYLE.yearInput}
        leadingZeros={4}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        -
      </span>
      <IntegerInput
        aria-label='Month' placeholder='MM'
        min={1} max={12}
        value={props.value?.month}
        readOnly={props.readonly}
        onChange={onMonthChange}
        style={STYLE.monthInput}
        leadingZeros={2}/>
      <span className={css(STYLES.separator)} style={separatorStyle}>
        -
      </span>
      <IntegerInput
        aria-label='Day' placeholder='DD'
        min={1} max={31}
        value={props.value?.day}
        readOnly={props.readonly}
        onChange={onDayChange}
        style={STYLE.dayInput}
        leadingZeros={2}/>
    </div>);
}

function today(): Beam.Date {
  const now = new Date();
  return new Beam.Date(now.getFullYear(), now.getMonth() + 1, now.getDate());
}

type DateLabelProperties = Omit<Properties, 'readonly' | 'onChange'>;

/** A read-only date display. */
export function DateLabel(props: DateLabelProperties): JSX.Element {
  return <DateInput value={props.value} readonly/>;
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
    width: '0.45em',
    textAlign: 'center',
    userSelect: 'none'
  }
});

const STYLE = {
  dayInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.45em'
  } as React.CSSProperties,
  monthInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '1.8em'
  } as React.CSSProperties,
  yearInput: {
    backgroundColor: 'transparent',
    border: 'none',
    padding: 0,
    textAlign: 'center',
    font: '400 14px Roboto',
    width: '2.6em'
  } as React.CSSProperties
};
